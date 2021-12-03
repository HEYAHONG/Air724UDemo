
#include "appmqtt.hpp"
#include "appstack.hpp"
#include "kconfig.h"
#include "debug.h"
#include "time.h"
#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
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

const int keepalive=10;

static void mqttmessageHandler(MessageData*msg)
{
    char * topic=(char *)iot_os_malloc(msg->topicName->lenstring.len+1);
    memset(topic,0,msg->topicName->lenstring.len+1);
    memcpy(topic,msg->topicName->lenstring.data,msg->topicName->lenstring.len);

    char  *payload =(char *)iot_os_malloc(msg->message->payloadlen+1);
    memset(payload,0,msg->message->payloadlen+1);
    memcpy(payload,msg->message->payload,msg->message->payloadlen);

    app_debug_print("%s:topic:%s,data=%s,datalen=%uBytes\r\n",TAG,topic,(char *)payload,msg->message->payloadlen);
    //MQTTPublish(&mqttclient,"/echo",msg->message);

    iot_os_free(topic);
    iot_os_free(payload);
}




static void mqtt_receive_task(void *arg)
{
    app_debug_print("%s:mqtt task start!!\r\n",TAG);
    while(true)
    {
        //测试MQTT连接
        app_debug_print("%s:mqtt test start!!\r\n",TAG);


        NetworkInit(&mqttserver);
        while(0!=NetworkConnect(&mqttserver,(char *)"didiyun.hyhsystem.cn",1883))
        {
            app_debug_print("%s:connect mqtt server!\r\n",TAG);
            iot_os_sleep(3000);
        }

        MQTTClientInit(&mqttclient,&mqttserver,3000,mqtttxbuff,sizeof(mqtttxbuff),mqttrxbuff,sizeof(mqttrxbuff));


        MQTTPacket_connectData cfg=MQTTPacket_connectData_initializer;

        //使用keepalive选项
        cfg.keepAliveInterval=keepalive;

        if(SUCCESS!=MQTTConnect(&mqttclient,&cfg))
        {
            app_debug_print("%s:mqtt connect failed!!\r\n",TAG);
            continue;
        }

        if(SUCCESS!=MQTTSubscribe(&mqttclient,"+/#",QOS0,mqttmessageHandler))
        {
            app_debug_print("%s:mqtt subscribe failed!!\r\n",TAG);
            continue;
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


        app_debug_print("%s:mqtt yield failed!!restarting!!!\r\n",TAG);

        if(mqttserver.disconnect!=NULL)
        {
            mqttserver.disconnect(&mqttserver);
        }

    }

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
        if(mqttclient.isconnected && iot_os_get_system_tick()-last_ping_tick> keepalive*1000/2/ms_per_tick )//ping 一次
        {
            last_ping_tick=iot_os_get_system_tick();
            bool is_ok=mqtt_ping(&mqttclient);
           app_debug_print("%s:ping %s\r\n",TAG,is_ok?"success":"failed");
        }
       iot_os_sleep(1000);
    }

}

void MQTT_Init()
{

    uint8_t pri=app_get_auto_task_priority();

    iot_os_create_task(mqtt_receive_task, NULL, 4096, pri, OPENAT_OS_CREATE_DEFAULT,(char *) "MQTT Receive");
    iot_os_create_task(mqtt_ping_task, NULL, 4096, pri, OPENAT_OS_CREATE_DEFAULT,(char *) "MQTT Ping");

}
