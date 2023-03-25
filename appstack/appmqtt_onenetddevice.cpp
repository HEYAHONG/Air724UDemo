#include "appmqtt_onenetddevice.h"
#include "debug.h"

#if CONFIG_MQTT_STACK_ONENET_DEVICE == 1
#include "onenettoken.h"
#include "onenetonejson.h"
#include <string>
#include <chrono>
#include "time.h"
#include "main.h"

static const char *TAG="MQTT_ONENET_DEVICE";

/*
产品ID
*/
#define ONENET_PRODUCT_ID "coo6ifUmjQ"
/*
产品访问Key,本代码采用产品Key+自动注册的方式
*/
#define ONENET_PRODUCT_ACCESS_KEY "vt/QsIKvWn7znHdzFVdexOUMsS9wXsNdXbKHVaZiyrQ="
/*
采用IMEI作为设备名称
*/
#define ONENET_PRODUCT_DEVICENAME get_imei()
/*
OneNET Token版本
*/
#define ONENET_TOKEN_VERSION "2018-10-31"
/*
OneNET Hmac method
*/
#define ONENET_TOKEN_METHOD "sha256"

static std::string OneNETDeviceGetRes(std::string productid,std::string devicename)
{
    return std::string("products/")+ productid +"/devices/"+devicename;
}

static std::string OneNETGetStringForSignature(std::string et,std::string res,std::string method=ONENET_TOKEN_METHOD,std::string version=ONENET_TOKEN_VERSION)
{
    return et+"\n"+method+"\n"+res+"\n"+version;
}

static std::string  OneNETDeviceGetSign(time_t et,std::string productid,std::string devicename,std::string key)
{
    std::string to_sign=OneNETGetStringForSignature(std::to_string(et),OneNETDeviceGetRes(productid,devicename));
    std::string sign;
    if(key.empty() || productid.empty() || devicename.empty())
    {
        return sign;
    }
    std::string key_bin;
    {
        uint8_t key_blob[key.length()]= {0};
        size_t key_blob_length=OneNETBase64Decode(key_blob,sizeof(key_blob),(uint8_t *)key.c_str(),key.length());
        if(key_blob_length == 0)
        {
            return sign;
        }
        key_bin=std::string((char *)key_blob,key_blob_length);
    }
    {
        uint8_t sign_blob[ONENET_HMAC_OUT_MAX]= {0};
        OneNETTokenCryptoMethod method=ONENET_CRYPTO_DEFAULT;
        if(std::string(ONENET_TOKEN_METHOD)=="md5")
        {
            method=ONENET_CRYPTO_MD5;
        }
        if(std::string(ONENET_TOKEN_METHOD)=="sha1")
        {
            method=ONENET_CRYPTO_SHA1;
        }
        if(std::string(ONENET_TOKEN_METHOD)=="sha256")
        {
            method=ONENET_CRYPTO_SHA256;
        }
        size_t sign_blob_length=OneNETHmac(method,(uint8_t *)sign_blob,sizeof(sign_blob),(uint8_t *) key_bin.c_str(),key_bin.length(),(uint8_t *)to_sign.c_str(),to_sign.length());
        if(sign_blob_length == 0)
        {
            return sign;
        }
        uint8_t sign_base64[ONENET_HMAC_OUT_MAX*4/3+4]= {0};
        OneNETBase64Encode(sign_base64,sizeof(sign_base64),sign_blob,sign_blob_length);
        sign=std::string((char *)sign_base64);
    }
    return sign;
}

static bool  StringReplace(std::string &val,std::string old_str,std::string new_str)
{
    std::string::size_type pos=val.find(old_str);
    if(pos==std::string::npos)
    {
        return false;
    }

    val.replace(pos,old_str.length(),new_str);

    return true;

}



static void OneNETTokenUrlConvert(std::string &val)
{
    while(StringReplace(val,"%","%25"));
    while(StringReplace(val,"+","%2B"));
    while(StringReplace(val," ","%20"));
    while(StringReplace(val,"/","%2F"));
    while(StringReplace(val,"?","%3F"));
    while(StringReplace(val,"#","%23"));
    while(StringReplace(val,"&","%26"));
    while(StringReplace(val,"=","%3D"));
}

static std::string  OneNETDeviceGetToken(time_t _et,std::string productid,std::string devicename,std::string key)
{
    std::string et=std::to_string(_et);
    std::string res= OneNETDeviceGetRes(productid,devicename);
    OneNETTokenUrlConvert(res);
    std::string sign=OneNETDeviceGetSign(_et,productid,devicename,key);
    if(sign.empty())
    {
        return std::string("");
    }
    OneNETTokenUrlConvert(sign);
    std::string version=ONENET_TOKEN_VERSION;
    OneNETTokenUrlConvert(version);
    std::string method=ONENET_TOKEN_METHOD;
    OneNETTokenUrlConvert(method);

    return  std::string("version=")+version+"&res="+res+"&et="+et+"&method="+method+"&sign="+sign;

}

static OneNETOneJson *onejson=NULL;

/*
此文件只包含常用功能，详细接口请查看onenetonejson.h的接口
*/

static std::map<std::string,Json::Value>  *PropertyCache=NULL;

/** \brief 属性值设置
 *
 * \param key std::string 属性名称
 * \param value Json::Value 属性值，可以为任意Json类型
 * \return bool 是否成功
 *
 */
static bool DeviceOnPropertySet(std::string key,Json::Value value)
{
    if(PropertyCache==NULL)
    {
        PropertyCache=new std::map<std::string,Json::Value>;
    }
    (*PropertyCache)[key]=value;
    return true;
}

/** \brief 属性值获取
 *
 * \param key std::string 属性名称
 * \param value Json::Value& 属性值，可以为任意Json类型
 * \return bool 是否成功
 *
 */
static bool DeviceOnPropertyGet(std::string key,Json::Value& value)
{
    if(PropertyCache==NULL)
    {
        PropertyCache=new std::map<std::string,Json::Value>;
    }
    if((*PropertyCache).find(key)!=(*PropertyCache).end())
    {
        value=(*PropertyCache)[key];
        return true;
    }
    return false;
}

void MQTT_OneNETDevice_Init(MQTT_Cfg_t &cfg)
{
    {
        const char *imei=get_imei();
        if(imei==NULL || strlen(imei)==0)
        {
            app_debug_print("%s:wait for imei!\n",TAG);
            return;
        }
    }
    {
        auto now=std::chrono::system_clock::now();
        if(std::chrono::duration_cast<std::chrono::days>(now.time_since_epoch()).count() < 30)
        {
            app_debug_print("%s:wait for time sync!\n",TAG);
            return;
        }
    }
    {
        std::string productid=ONENET_PRODUCT_ID;
        std::string productkey=ONENET_PRODUCT_ACCESS_KEY;
        std::string devicename=get_imei();
        auto now=std::chrono::system_clock::now();
        now+=std::chrono::hours(1);//一小时有效期
        time_t end_time_t=std::chrono::system_clock::to_time_t(now);
        std::string token=OneNETDeviceGetToken(end_time_t,productid,devicename,productkey);
        if(token.empty())
        {
            app_debug_print("%s:token clac error!\n",TAG);
            return;
        }

        cfg.keepalive=CONFIG_MQTT_STACK_ONENET_DEVICE_KEEPALIVE;
        cfg.clientid=devicename;
        cfg.auth.username=productid;
        cfg.auth.password=token;
        cfg.cleansession=true;
        //订阅所有设备主题
        cfg.subscribe.subtopic=(std::string("$sys/")+productid+"/"+devicename+"/#");
        cfg.subscribe.qos=0;

    }

    if(onejson==NULL)
    {
        onejson=new OneNETOneJson();

        //设置设备信息
        onejson->SetDev(ONENET_PRODUCT_ID,ONENET_PRODUCT_DEVICENAME);
        //设置MQTT发送
        onejson->SetMQTTPublish([](std::string topic,std::string payload)
        {
            MQTT_Message_Ptr_t msg=std::make_shared<MQTT_Message_t>();
            msg->topic=topic;
            msg->payload=payload;
            msg->qos=0;
            msg->retain=0;
            return MQTT_Publish_Message(msg);
        });
        //设置设备回调
        onejson->SetOnPropertyGet(DeviceOnPropertyGet);
        onejson->SetOnPropertySet(DeviceOnPropertySet);
    }

}

void MQTT_OneNETDevice_Connect(MQTT_Cfg_t &cfg)
{
    app_debug_print("%s:OneNET Device Online\n",TAG);
}

void MQTT_OneNETDevice_DisConnect(MQTT_Cfg_t &cfg)
{
    //清空clientid以重新初始化(重新计算Token)
    cfg.clientid="";

}



void MQTT_OneNETDevice_OnMessage(MQTT_Cfg_t &cfg,MQTT_Message_Ptr_t msg)
{
    app_debug_print("%s:OneNETMessage:topic=%s\nmessage=\n%s\n",TAG,msg->topic.c_str(),msg->payload.c_str());
    if(onejson!=NULL)
    {
        try
        {
            onejson->OnMQTTMessage(msg->topic,msg->payload);
        }
        catch(...)
        {

        }
    }

}

#endif
