
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

const int keepalive=120;

extern SMGS_gateway_context_t gateway_context;

static void mqttmessageHandler(MessageData*msg)
{
    uint8_t buff[4096]= {0};
    SMGS_GateWay_Receive_MQTT_MSG(&gateway_context,msg->topicName->lenstring.data,msg->topicName->lenstring.len,(uint8_t *)msg->message->payload,msg->message->payloadlen,msg->message->qos,msg->message->retained,buff,sizeof(buff));
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
    app_debug_print("%s:Device_Command(CmdID=%04X)\r\n",TAG,(uint32_t)(*cmdid));
    return _ret;
}

bool SMGS_Device_ReadRegister(SMGS_device_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_register_address_t addr,uint64_t *dat,SMGS_payload_register_flag_t *flag)
{
    bool ret=false;
    app_debug_print("%s:Device_ReadRegister(Addr=%04X)\r\n",TAG,(uint32_t)addr);
    return ret;
}

bool SMGS_Device_WriteRegister(SMGS_device_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_register_address_t addr,uint64_t *dat,SMGS_payload_register_flag_t *flag)
{
    bool ret=false;
    app_debug_print("%s:Device_WriteRegister(Addr=%04X,Data=%016llX,Flag=%02X)\r\n",TAG,(uint32_t)addr,(*dat),(uint32_t)(flag->val));
    return ret;
}

bool SMGS_Device_ReadSensor(SMGS_device_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_sensor_address_t addr,uint64_t *dat,SMGS_payload_sensor_flag_t *flag)
{
    bool ret=false;
    app_debug_print("%s:Device_ReadSensor(Addr=%04X,Flag=%02X)\r\n",TAG,(uint32_t)addr,(uint32_t)(flag->val));
    return ret;
}



SMGS_gateway_context_t gateway_context;

bool SMGS_GateWay_Command(SMGS_gateway_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_cmdid_t *cmdid,uint8_t *cmddata,size_t cmddata_length,uint8_t *retbuff,size_t *retbuff_length,SMGS_payload_retcode_t *ret)
{
    bool _ret=false;
    app_debug_print("%s:GateWay_Command(CmdID=%04X)\r\n",TAG,(uint32_t)(*cmdid));
    return _ret;
}

bool SMGS_GateWay_ReadRegister(SMGS_gateway_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_register_address_t addr,uint64_t *dat,SMGS_payload_register_flag_t *flag)
{
    bool ret=false;
    app_debug_print("%s:GateWay_ReadRegister(Addr=%04X)\r\n",TAG,(uint32_t)addr);
    return ret;
}

bool SMGS_GateWay_WriteRegister(SMGS_gateway_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_register_address_t addr,uint64_t *dat,SMGS_payload_register_flag_t *flag)
{
    bool ret=false;
    app_debug_print("%s:GateWay_WriteRegister(Addr=%04X,Data=%016llX,Flag=%02X)\r\n",TAG,(uint32_t)addr,(*dat),(uint32_t)(flag->val));
    return ret;
}

bool SMGS_GateWay_ReadSensor(SMGS_gateway_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_sensor_address_t addr,uint64_t *dat,SMGS_payload_sensor_flag_t *flag)
{
    bool ret=false;
    app_debug_print("%s:GateWay_ReadSensor(Addr=%04X,Flag=%02X)\r\n",TAG,(uint32_t)addr,(uint32_t)(flag->val));
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
    if(MQTTIsConnected(&mqttclient)==0)
    {
        return false;
    }

    QoS Qos=QOS0;
    switch(qos)
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

    MQTTMessage msg;
    memset(&msg,0,sizeof(msg));
    msg.payload=payload;
    msg.payloadlen=payloadlen;
    msg.qos=Qos;
    msg.retained=retain;
    return MQTTPublish(&mqttclient,topic,&msg)==0;
}

extern  "C" const char * get_imei();
static char subscribestr[64];
static void mqtt_receive_task(void *arg)
{


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


    app_debug_print("%s:mqtt task start!!\r\n",TAG);
    while(true)
    {
        //测试MQTT连接
        app_debug_print("%s:mqtt test start!!\r\n",TAG);


        NetworkInit(&mqttserver);
        while(0!=NetworkConnect(&mqttserver,(char *)"mqtt.hyhsystem.cn",1883))
        {
            app_debug_print("%s:connect mqtt server!\r\n",TAG);
            iot_os_sleep(3000);
        }

        MQTTClientInit(&mqttclient,&mqttserver,3000,mqtttxbuff,sizeof(mqtttxbuff),mqttrxbuff,sizeof(mqttrxbuff));


        {

            MQTTPacket_connectData cfg=MQTTPacket_connectData_initializer;

            //使用keepalive选项
            cfg.keepAliveInterval=keepalive;

            //填写clientID
            cfg.clientID.cstring=(char *)GateWaySerialNumber;

            //填写cleansession
            cfg.cleansession=1;

            //填写用户名与密码
            cfg.username.cstring=(char *)GateWaySerialNumber;
            cfg.password.cstring=(char *)GateWaySerialNumber;

            //填写will
            uint8_t willbuff[256]= {0};
            SMGS_gateway_will_t will= {0};
            SMGS_GateWay_Will_Encode(&gateway_context,&will,willbuff,sizeof(willbuff));

            cfg.will.topicName.cstring=(char *)will.topic;
            cfg.will.qos=will.qos;
            cfg.will.message.lenstring.data=(char *)will.payload;
            cfg.will.message.lenstring.len=will.payloadlen;
            cfg.will.retained=will.ratain;
            cfg.willFlag=1;


            if(SUCCESS!=MQTTConnect(&mqttclient,&cfg))
            {
                mqttserver.disconnect(&mqttserver);
                app_debug_print("%s:mqtt connect failed!!\r\n",TAG);
                continue;
            }
        }
        {


            memset(subscribestr,0,sizeof(subscribestr));
            strcat(subscribestr,GateWaySerialNumber);
            strcat(subscribestr,"/#");

            if(SUCCESS!=MQTTSubscribe(&mqttclient,subscribestr,QOS0,mqttmessageHandler))
            {
                mqttserver.disconnect(&mqttserver);
                app_debug_print("%s:mqtt subscribe failed!!\r\n",TAG);
                continue;
            }
        }

        {
            //发送网关上线消息
            uint8_t buff[512]= {0};
            SMGS_GateWay_Online(&gateway_context,buff,sizeof(buff),0,0);
        }

         app_debug_print("%s:SimpleMQTTGateWayStack Online\r\n",TAG);


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


HANDLE mqtt_receive_task_handle=NULL;
HANDLE mqtt_ping_task_handle=NULL;
void MQTT_Init()
{
    if(mqtt_receive_task_handle!=NULL &&mqtt_ping_task_handle !=NULL)
    {
        return;
    }
    else
    {
        app_debug_print("%s:Init!\r\n",TAG);
    }
    uint8_t pri=app_get_auto_task_priority();
    mqtt_receive_task_handle=iot_os_create_task(mqtt_receive_task, NULL, 6144, pri, OPENAT_OS_CREATE_DEFAULT,(char *) "MQTT Receive");
    mqtt_ping_task_handle=iot_os_create_task(mqtt_ping_task, NULL, 2048, pri, OPENAT_OS_CREATE_DEFAULT,(char *) "MQTT Ping");

}
