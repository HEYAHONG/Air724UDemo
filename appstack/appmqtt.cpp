
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
#include "RC.h"

#ifdef __cplusplus
};
#endif // __cplusplus

extern "C"
{
#include "MQTTClient.h"
    extern void * cpp_malloc(size_t nsize);
    extern void cpp_free(void *p);
#include "debug.h"
#include "string.h"
}
static const char * TAG="MQTT";


/*
MQTT相关
*/
struct Network mqttserver= {0};
struct MQTTClient mqttclient= {0};
uint8_t mqtttxbuff[1024]= {0};
uint8_t mqttrxbuff[1024]= {0};
static std::shared_ptr<MQTT_Cfg_t> Cfg=NULL;
static std::shared_ptr<MQTT_Callback_t> callback=NULL;
void MQTT_Set_Callback(MQTT_Callback_t cb)
{
    (*callback)=cb;
}

bool MQTT_Publish_Message(MQTT_Message_Ptr_t msg)
{
    auto client=&mqttclient;
    if(client==NULL || client->buf ==NULL || client->buf_size ==0 || client->ipstack==NULL || client->ipstack->mqttwrite==NULL)
    {
        return false;//参数有误
    }
    if(MQTTIsConnected(&mqttclient)==0)
    {
        return false;
    }

    QoS Qos=QOS0;
    switch(msg->qos)
    {
    default:
        break;
    case 0:
        Qos=QOS0;
        break;
    case 1:
        Qos=QOS1;
        break;
    case 2:
        Qos=QOS2;
        break;

    }

    struct MQTTMessage Msg;
    memset(&Msg,0,sizeof(Msg));
    Msg.payload=(char *)msg->payload.c_str();
    Msg.payloadlen=msg->payload.length();
    Msg.qos=Qos;
    Msg.retained=msg->retain;
    return MQTTPublish(&mqttclient,msg->topic.c_str(),&Msg)==0;
}
static bool check_cfg(MQTT_Cfg_t &Cfg)
{
    if(Cfg.host.empty())
    {
        return false;
    }

    if(Cfg.port==0)
    {
        return false;
    }

    if(Cfg.keepalive==0)
    {
        Cfg.keepalive=120;
    }

    if(Cfg.clientid.empty())
    {
        return false;
    }

    return true;
}

static void mqttmessageHandler(MessageData*msg)
{
    MQTT_Message_Ptr_t ptr=std::make_shared<MQTT_Message_t>();
    ptr->topic=std::string(msg->topicName->lenstring.data,msg->topicName->lenstring.len);
    ptr->payload=std::string((char *)msg->message->payload,msg->message->payloadlen);
    ptr->qos=(uint8_t)msg->message->qos;
    ptr->retain=msg->message->retained;

    if(callback->onmessage!=NULL)
    {
        callback->onmessage(*Cfg,ptr);
    }
}

static void mqtt_receive_task(void *arg)
{
    app_debug_print("%s:mqtt task start!!\r\n",TAG);
    while(true)
    {

        while(!check_cfg(*Cfg))
        {
            if(callback->init!=NULL)
            {
                app_debug_print("%s:wait for config!!\r\n",TAG);
                callback->init(*Cfg);
                iot_os_sleep(500);
            }
            else
            {
                app_debug_print("%s:mqtt not init!!\r\n",TAG);
                iot_os_sleep(3000);
            }
        }

        app_debug_print("%s:mqtt start!!\r\n",TAG);
        NetworkInit(&mqttserver);
#if CONFIG_MQTT_SSL == 1
        {
            mqttserver.cacert=Cfg->ssl.cacert.c_str();
            mqttserver.cacertlen=Cfg->ssl.cacert.length();
        }
#endif // CONFIG_MQTT_SSL
        while(0!=NetworkConnect(&mqttserver,(char *)Cfg->host.c_str(),Cfg->port))
        {
            app_debug_print("%s:connect mqtt server!\r\n",TAG);
            iot_os_sleep(3000);
        }

        MQTTClientInit(&mqttclient,&mqttserver,3000,mqtttxbuff,sizeof(mqtttxbuff),mqttrxbuff,sizeof(mqttrxbuff));

        {

            MQTTPacket_connectData cfg=MQTTPacket_connectData_initializer;

            //使用keepalive选项
            cfg.keepAliveInterval=Cfg->keepalive;;

            //填写clientID
            cfg.clientID.cstring=(char *)Cfg->clientid.c_str();

            //填写cleansession
            cfg.cleansession=Cfg->cleansession;

            //填写用户名与密码
            if(!Cfg->auth.username.empty())
                cfg.username.cstring=(char *)Cfg->auth.username.c_str();
            if(!Cfg->auth.password.empty())
                cfg.password.cstring=(char *)Cfg->auth.password.c_str();

            if(!Cfg->will.will_topic.empty())
            {
                cfg.willFlag=1;
                cfg.will.topicName.cstring=(char *)Cfg->will.will_topic.c_str();
                cfg.will.message.lenstring.data=(char *)Cfg->will.will_payload.c_str();
                cfg.will.message.lenstring.len=Cfg->will.will_payload.length();
                cfg.will.qos=Cfg->will.will_qos;
                cfg.will.retained=Cfg->will.will_retain;
            }

            if(SUCCESS!=MQTTConnect(&mqttclient,&cfg))
            {
                if(callback->disconnect!=NULL)
                {
                    callback->disconnect(*Cfg);
                }
                mqttserver.disconnect(&mqttserver);
                app_debug_print("%s:mqtt connect failed!!\r\n",TAG);
                continue;
            }
        }

        {
            if(!Cfg->subscribe.subtopic.empty())
            {
                QoS Qos=QOS0;
                switch(Cfg->subscribe.qos)
                {
                default:
                    break;
                case 0:
                    Qos=QOS0;
                    break;
                case 1:
                    Qos=QOS1;
                    break;
                case 2:
                    Qos=QOS2;
                    break;

                }
                if(SUCCESS!=MQTTSubscribe(&mqttclient,Cfg->subscribe.subtopic.c_str(),Qos,mqttmessageHandler))
                {
                    mqttserver.disconnect(&mqttserver);
                    app_debug_print("%s:mqtt subscribe failed!!\r\n",TAG);
                    continue;
                }
            }
        }



        {
            if(callback->connect!=NULL)
            {
                callback->connect(*Cfg);
            }
        }

        {
            while(true)
            {
                if(SUCCESS!=MQTTYield(&mqttclient,10))
                {
                    break;
                }
                iot_os_sleep(1);
            }
        }

        if(callback->disconnect!=NULL)
        {
            callback->disconnect(*Cfg);
        }

        app_debug_print("%s:mqtt yield failed!!restarting!!!\r\n",TAG);

        if(mqttserver.disconnect!=NULL)
        {
            mqttserver.disconnect(&mqttserver);
        }

    }

}


bool MQTT_Is_Connected()
{
    return MQTTIsConnected(&mqttclient)!=0;
}



//执行ping
static bool mqtt_ping(MQTTClient * client)
{

    if(client==NULL || client->buf ==NULL || client->buf_size ==0 || client->ipstack==NULL || client->ipstack->mqttwrite==NULL)
    {
        return false;//参数有误
    }

    uint8_t buff[8]= {0};
    int len=MQTTSerialize_pingreq(buff,sizeof(buff));
    return len==client->ipstack->mqttwrite(client->ipstack,buff,len,1000);
}

static void mqtt_ping_task(void *arg)
{

    uint64_t last_ping_tick=iot_os_get_system_tick();
    while(true)
    {
        if(mqttclient.isconnected && iot_os_get_system_tick()-last_ping_tick> Cfg->keepalive*1000/2/ms_per_tick )//ping 一次
        {
            last_ping_tick=iot_os_get_system_tick();
            bool is_ok=mqtt_ping(&mqttclient);
            app_debug_print("%s:ping %s\r\n",TAG,is_ok?"success":"failed");
        }
        iot_os_sleep(1000);
    }

}


HANDLE mqtt_receive_task_handle=NULL;
HANDLE mqtt_ping_task_handle=NULL;
#include "appmqtt_smgs.h"
#include "appmqtt_onenetddevice.h"
void MQTT_Init()
{
    if(mqtt_receive_task_handle!=NULL &&mqtt_ping_task_handle !=NULL)
    {
        return;
    }
    else
    {
        //变量初始化
        Cfg=std::make_shared<MQTT_Cfg_t>();
        callback=std::make_shared<MQTT_Callback_t>();


        MQTT_Callback_t cb= {NULL,NULL,NULL,NULL};
        MQTT_Set_Callback(cb);

#if CONFIG_MQTT_STACK_SMGS == 1
        cb.init=MQTT_SMGS_Init;
        cb.connect=MQTT_SMGS_Connect;
        cb.disconnect=MQTT_SMGS_DisConnect;
        cb.onmessage=MQTT_SMGS_OnMessage;
        MQTT_Set_Callback(cb);
        {
            Cfg->host=CONFIG_MQTT_HOST;
            Cfg->port=CONFIG_MQTT_PORT;
#if CONFIG_MQTT_SSL == 1
            {
                const char * rc=(char *)RCGetHandle(CONFIG_MQTT_SSL_CERT_CA);
                if(rc)
                {
                    Cfg->ssl.cacert=std::string(rc);
                }
            }
#endif // CONFIG_MQTT_SSL
        }
#endif // CONFIG_MQTT_STACK_SMGS

#if CONFIG_MQTT_STACK_ONENET_DEVICE == 1
        cb.init=MQTT_OneNETDevice_Init;
        cb.connect=MQTT_OneNETDevice_Connect;
        cb.disconnect=MQTT_OneNETDevice_DisConnect;
        cb.onmessage=MQTT_OneNETDevice_OnMessage;
        MQTT_Set_Callback(cb);
        {
            Cfg->host=CONFIG_MQTT_ONENET_HOST;
            Cfg->port=CONFIG_MQTT_ONENET_PORT;
#if CONFIG_MQTT_SSL == 1
            {
                const char * rc=(char *)RCGetHandle(CONFIG_MQTT_ONENET_SSL_CERT_CA);
                if(rc)
                {
                    Cfg->ssl.cacert=std::string(rc);
                }
            }
#endif // CONFIG_MQTT_SSL
        }
#endif // CONFIG_MQTT_STACK_ONENET_DEVICE

        app_debug_print("%s:Init!\r\n",TAG);
    }
    uint8_t pri=app_get_auto_task_priority();
    /*
    如需启用SSL,需要较大任务栈
    */
    mqtt_receive_task_handle=iot_os_create_task(mqtt_receive_task, NULL, 8192, pri, OPENAT_OS_CREATE_DEFAULT,(char *) "MQTT Receive");
    mqtt_ping_task_handle=iot_os_create_task(mqtt_ping_task, NULL, 8192, pri, OPENAT_OS_CREATE_DEFAULT,(char *) "MQTT Ping");

}
