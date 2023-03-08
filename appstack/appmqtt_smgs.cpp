#include "appmqtt_smgs.h"
#include "appmqtt.hpp"
#include "appstack.hpp"
#include "kconfig.h"
#include "debug.h"
#include "time.h"
#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "libSMGS.h"
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "iot_os.h"

#ifdef __cplusplus
};
#endif // __cplusplus

extern "C"
{
#include "MQTTClient.h"
#include "debug.h"
#include "string.h"
}

#if CONFIG_MQTT_STACK_SMGS == 1

static const char * TAG="MQTT_SMGS";

extern SMGS_gateway_context_t gateway_context;

void MQTT_SMGS_OnMessage(MQTT_Cfg_t &cfg,MQTT_Message_Ptr_t msg)
{

    size_t buff_len=4096;
    uint8_t *buff=new uint8_t[buff_len];
    memset(buff,0,buff_len);
    SMGS_GateWay_Receive_MQTT_MSG(&gateway_context,msg->topic.c_str(),msg->topic.length(),(uint8_t *)msg->payload.c_str(),msg->payload.length(),msg->qos,msg->retain,buff,buff_len);
    delete [] buff;
}


/*
协议栈相关
*/
static const char * GateWayName="Air724Demo";
static char GateWaySerialNumber[32]="A724";

SMGS_device_context_t device_context;

bool  SMGS_Device_IsOnline(SMGS_device_context_t *ctx)
{
    //默认返回真
    return true;
}

bool SMGS_Device_Command(SMGS_device_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_cmdid_t *cmdid,uint8_t *cmddata,size_t cmddata_length,uint8_t *retbuff,size_t *retbuff_length,SMGS_payload_retcode_t *ret)
{
    bool _ret=false;
    app_debug_print("%s:Device_Command(CmdID=%04lX)\r\n",TAG,(uint32_t)(*cmdid));
    return _ret;
}

bool SMGS_Device_ReadRegister(SMGS_device_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_register_address_t addr,uint64_t *dat,SMGS_payload_register_flag_t *flag)
{
    bool ret=false;
    app_debug_print("%s:Device_ReadRegister(Addr=%04lX)\r\n",TAG,(uint32_t)addr);
    return ret;
}

bool SMGS_Device_WriteRegister(SMGS_device_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_register_address_t addr,uint64_t *dat,SMGS_payload_register_flag_t *flag)
{
    bool ret=false;
    app_debug_print("%s:Device_WriteRegister(Addr=%04lX,Data=%016llX,Flag=%02lX)\r\n",TAG,(uint32_t)addr,(*dat),(uint32_t)(flag->val));
    return ret;
}

bool SMGS_Device_ReadSensor(SMGS_device_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_sensor_address_t addr,uint64_t *dat,SMGS_payload_sensor_flag_t *flag)
{
    bool ret=false;
    app_debug_print("%s:Device_ReadSensor(Addr=%04lX,Flag=%02lX)\r\n",TAG,(uint32_t)addr,(uint32_t)(flag->val));
    return ret;
}



SMGS_gateway_context_t gateway_context= {0};

bool SMGS_GateWay_Command(SMGS_gateway_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_cmdid_t *cmdid,uint8_t *cmddata,size_t cmddata_length,uint8_t *retbuff,size_t *retbuff_length,SMGS_payload_retcode_t *ret)
{
    bool _ret=false;
    app_debug_print("%s:GateWay_Command(CmdID=%04lX)\r\n",TAG,(uint32_t)(*cmdid));
    return _ret;
}

bool SMGS_GateWay_ReadRegister(SMGS_gateway_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_register_address_t addr,uint64_t *dat,SMGS_payload_register_flag_t *flag)
{
    bool ret=false;
    app_debug_print("%s:GateWay_ReadRegister(Addr=%04lX)\r\n",TAG,(uint32_t)addr);
    return ret;
}

bool SMGS_GateWay_WriteRegister(SMGS_gateway_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_register_address_t addr,uint64_t *dat,SMGS_payload_register_flag_t *flag)
{
    bool ret=false;
    app_debug_print("%s:GateWay_WriteRegister(Addr=%04lX,Data=%016llX,Flag=%02lX)\r\n",TAG,(uint32_t)addr,(*dat),(uint32_t)(flag->val));
    return ret;
}

bool SMGS_GateWay_ReadSensor(SMGS_gateway_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_sensor_address_t addr,uint64_t *dat,SMGS_payload_sensor_flag_t *flag)
{
    bool ret=false;
    app_debug_print("%s:GateWay_ReadSensor(Addr=%04lX,Flag=%02lX)\r\n",TAG,(uint32_t)addr,(uint32_t)(flag->val));
    return ret;
}


//设备查询函数
SMGS_device_context_t * SMGS_Device_Next(struct __SMGS_gateway_context_t *ctx,SMGS_device_context_t * devctx)
{
    if(devctx==NULL)
    {
        return &device_context;//返回第一个设备
    }

    //由于只有一个设备，直接返回NULL

    return NULL;
}


static bool SMGS_MessagePublish(struct __SMGS_gateway_context_t *ctx,const char * topic,void * payload,size_t payloadlen,uint8_t qos,int retain)
{
    MQTT_Message_Ptr_t ptr=std::make_shared<MQTT_Message_t>();
    ptr->topic=std::string(topic);
    ptr->payload=std::string((char *)payload,payloadlen);
    ptr->qos=qos;
    ptr->retain=retain;
    return MQTT_Publish_Message(ptr);
}
extern  "C" const char * get_imei();
void MQTT_SMGS_Init(MQTT_Cfg_t &cfg)
{
    if(SMGS_Is_GateWay_Context_OK(&gateway_context))
    {
        return;
    }
    {
        //处理序列号
        const char * IMEI=get_imei();

        while(IMEI==NULL)
        {
            IMEI=get_imei();
            iot_os_sleep(1000);
        }

        size_t IMEI_length=strlen(IMEI);
        {
            strcat(GateWaySerialNumber,&IMEI[IMEI_length-12]);
        }
    }

    {
        cfg.keepalive=CONFIG_MQTT_STACK_SMGS_KEEPALIVE;
        cfg.clientid=GateWaySerialNumber;
        cfg.auth.username=GateWaySerialNumber;
        cfg.auth.password=GateWaySerialNumber;
        cfg.cleansession=true;
        cfg.subscribe.subtopic=(std::string(GateWaySerialNumber)+"/#");
        cfg.subscribe.qos=0;

    }

    {
        //初始化设备上下文
        SMGS_Device_Context_Init(&device_context);

        //填写设备上下文
        device_context.DeviceName=GateWayName;
        device_context.DevicePosNumber=1;
        device_context.DeviceSerialNumber=GateWaySerialNumber;//默认序列号同网关
        device_context.IsOnline=SMGS_Device_IsOnline;
        device_context.Command=SMGS_Device_Command;
        device_context.ReadRegister=SMGS_Device_ReadRegister;
        device_context.WriteRegister=SMGS_Device_WriteRegister;
        device_context.ReadSensor=SMGS_Device_ReadSensor;

    }

    {

        //初始化网关上下文
        SMGS_GateWay_Context_Init(&gateway_context,GateWaySerialNumber,SMGS_MessagePublish);

        //填写网关上下文
        gateway_context.GateWayName=GateWayName;
        gateway_context.Command=SMGS_GateWay_Command;
        gateway_context.ReadRegister=SMGS_GateWay_ReadRegister;
        gateway_context.WriteRegister=SMGS_GateWay_WriteRegister;
        gateway_context.ReadSensor=SMGS_GateWay_ReadSensor;
        gateway_context.Device_Next=SMGS_Device_Next;
    }

    {
        //填写will
        uint8_t willbuff[256]= {0};
        SMGS_gateway_will_t will= {0};
        SMGS_GateWay_Will_Encode(&gateway_context,&will,willbuff,sizeof(willbuff));

        cfg.will.will_topic=will.topic;
        cfg.will.will_payload=std::string((char *)will.payload,will.payloadlen);
        cfg.will.will_qos=will.qos;
        cfg.will.will_retain=will.ratain;
    }
}


void MQTT_SMGS_Connect(MQTT_Cfg_t &cfg)
{

    {
        //发送网关上线消息
        uint8_t buff[512]= {0};
        SMGS_GateWay_Online(&gateway_context,buff,sizeof(buff),0,0);
    }

    app_debug_print("%s:SimpleMQTTGateWayStack Online\r\n",TAG);


}

void MQTT_SMGS_DisConnect(MQTT_Cfg_t &cfg)
{

}

#endif
