#include "appmqtt.hpp"
#include "config.h"
#include "debug.h"
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


#include "stdint.h"
#include "stdlib.h"
#include "string.h"

//防止MQTTLib报错
#ifndef __INT8_T__
#define __INT8_T__
#endif // __INT8_T__

#ifndef __INT32_T__
#define __INT32_T__
#endif // __INT32_T__

#ifndef __UINT32_T__
#define __UINT32_T__
#endif // __UINT32_T__

#include "mqttlib.h"

#ifdef __cplusplus
};
#endif // __cplusplus

static __unused const char * TAG="appmqtt";

namespace default_mqtt
{
char *clientid=(char *)CONFIG_APPMQTT_DEFAULT_CLIENTID;
char *username=(char *)CONFIG_APPMQTT_DEFAULT_USERNAME;
char *password=(char *)CONFIG_APPMQTT_DEFAULT_PASSWORD;
char *will_topic=(char *)CONFIG_APPMQTT_DEFAULT_WILL_TOPIC;
char *will_payload=(char *)CONFIG_APPMQTT_DEFAULT_WILL_PAYLOAD;
}

MQTTConnectInfo::~MQTTConnectInfo()
{
    if(clientid!=default_mqtt::clientid)
    {
        iot_os_free(clientid);
    }
    if(username!=default_mqtt::username)
    {
        iot_os_free(username);
    }
    if(password!=default_mqtt::password)
    {
        iot_os_free(password);
    }
    if(will_topic!=default_mqtt::will_topic)
    {
        iot_os_free(will_topic);
    }
    will_payload_length=0;
    if(will_payload!=default_mqtt::will_payload)
    {
        iot_os_free(will_payload);
    }
}

MQTTConnectInfo::MQTTConnectInfo()
{
    clientid=default_mqtt::clientid;
    username=default_mqtt::username;
    password=default_mqtt::password;
    will_topic=default_mqtt::will_topic;
    will_payload=default_mqtt::will_payload;
    will_payload_length=strlen((char *)will_payload);
    flags=MQTT_CONNECT_FLAG_CLEAN;
    keepalive=120;
}

MQTTConnectInfo::MQTTConnectInfo(MQTTConnectInfo &other)
{
    if(&other!=this)
    {
        keepalive=other.keepalive;
        flags=other.flags;

        clientid=(char *)iot_os_malloc(strlen(other.clientid)+1);
        memset(clientid,0,strlen(other.clientid)+1);
        memcpy(clientid,other.clientid,strlen(other.clientid));

        username=(char *)iot_os_malloc(strlen(other.username)+1);
        memset(username,0,strlen(other.username)+1);
        memcpy(username,other.username,strlen(other.username));

        password=(char *)iot_os_malloc(strlen(other.password)+1);
        memset(password,0,strlen(other.password)+1);
        memcpy(password,other.password,strlen(other.password));

        will_topic=(char *)iot_os_malloc(strlen(other.will_topic)+1);
        memset(will_topic,0,strlen(other.will_topic)+1);
        memcpy(will_topic,other.will_topic,strlen(other.will_topic));

        will_payload_length=other.will_payload_length;
        will_payload=iot_os_malloc(will_payload_length);
        memcpy(will_payload,other.will_payload,will_payload_length);

    }
}

void MQTTConnectInfo::set_flags(int _flags)
{
    flags=_flags;
}
void MQTTConnectInfo::set_keepalive(uint16_t _keepalive)
{
    keepalive=_keepalive;
}
bool MQTTConnectInfo::set_username_and_password(char *_username,char *_password)
{
    if(_username==NULL || password ==NULL)
    {
        return false;
    }
    {
        char *old_username=username;
        char * new_username=(char *)iot_os_malloc(strlen(_username)+1);
        memset(new_username,0,strlen(_username)+1);
        memcpy(new_username,_username,strlen(_username));
        username=new_username;
        iot_os_free(old_username);
    }

    {
        char *old_password=password;
        char * new_password=(char *)iot_os_malloc(strlen(_password)+1);
        memset(new_password,0,strlen(_password)+1);
        memcpy(new_password,_password,strlen(_password));
        password=new_password;
        iot_os_free(old_password);
    }
    return true;
}
bool MQTTConnectInfo::set_willdata(char * _will_topic,void * _will_payload,size_t _will_payload_length)
{
    if(_will_payload==NULL || _will_topic ==NULL)
    {
        return false;
    }
    {
        char *old_will_topic=will_topic;
        char * new_will_topic=(char *)iot_os_malloc(strlen(_will_topic)+1);
        memset(new_will_topic,0,strlen(_will_topic)+1);
        memcpy(new_will_topic,_will_topic,strlen(_will_topic));
        will_topic=new_will_topic;
        iot_os_free(old_will_topic);
    }
    {
        char *old_will_payload=(char *)will_payload;
        char * new_will_payload=(char *)iot_os_malloc(_will_payload_length);
        memcpy(new_will_payload,_will_payload,_will_payload_length);
        will_payload=new_will_payload;
        will_payload_length=_will_payload_length;
        iot_os_free(old_will_payload);
    }

    return true;
}

//构造函数
MQTT::MQTT(MQTTConnectInfo  & _connectinfo,size_t MaxTxBuffSize,size_t MaxRxBuffSize,size_t MaxPayloadBuffSize)
{
    connectinfo=_connectinfo;
    TxBuffSize=MaxTxBuffSize;
    if(TxBuffSize<CONFIG_APPMQTT_MAX_TXBUFFSIZE)
    {
        TxBuffSize=CONFIG_APPMQTT_MAX_TXBUFFSIZE;
    }
    RxBuffSize=MaxRxBuffSize;
    if(RxBuffSize<CONFIG_APPMQTT_MAX_RXBUFFSIZE)
    {
        RxBuffSize=CONFIG_APPMQTT_MAX_RXBUFFSIZE;
    }
    PayloadBuffSize=MaxPayloadBuffSize;
    if(PayloadBuffSize<CONFIG_APPMQTT_MAX_PAYLOADBUFFSIZE)
    {
        PayloadBuffSize=CONFIG_APPMQTT_MAX_PAYLOADBUFFSIZE;
    }

    TxBuff=(char *)iot_os_malloc(TxBuffSize);

    RxBuff=(char *)iot_os_malloc(RxBuffSize);

    PayloadBuff=(char *)iot_os_malloc(PayloadBuffSize);


    connectstate.appsocketid=-1;
    connectstate.isconnected=false;
    connectstate.socketfd=-1;
}

bool MQTT::connect(const char *ip,uint16_t port)
{
    if(ip==NULL || port ==0)
    {
        return false;
    }

    app_debug_print("%s:start connecting !!!\n\r",TAG);

    disconnect();

    appsocket_cfg_t cfg= {0};

    cfg.server_addr=appsocket_get_addr_by_ip(ip,port);

    cfg.userptr=this;//传递this指针

    cfg.before_connect=appsocket_before_connect;
    cfg.after_connect=appsocket_after_connect;
    cfg.onloop=appsocket_onloop;
    cfg.before_close=appsocket_before_close;

    connectstate.appsocketid=appsocket_add(cfg);

    if(connectstate.appsocketid<0)
    {
        return false;
    }

    return true;
}

void MQTT::disconnect()
{
    if(connectstate.appsocketid>=0)
    {
        appsocket_remove(connectstate.appsocketid);
        connectstate.socketfd=-1;
        connectstate.appsocketid=-1;
    }
}

bool MQTT::get_is_connected()
{
    return connectstate.isconnected;
}

//连接前回调函数
void MQTT::appsocket_before_connect(const struct __appsocket_cfg_t * cfg,int socket_fd)
{
    MQTT &m=*(MQTT *)cfg->userptr;
    m.connectstate.isconnected=false;
    m.connectstate.socketfd=socket_fd;
}
//连接成功后回调函数
void MQTT::appsocket_after_connect(const struct __appsocket_cfg_t *cfg,int socketfd)
{
    MQTT &m=*(MQTT *)cfg->userptr;

    Buffer_Struct TxBuffer= {0};
    TxBuffer.Data=(uint8_t *)m.TxBuff;
    TxBuffer.Pos=0;
    TxBuffer.MaxLen=m.TxBuffSize;
    Buffer_Struct PayloadBuffer= {0};
    PayloadBuffer.Pos=0;
    PayloadBuffer.Data=(uint8_t *)m.PayloadBuff;
    PayloadBuffer.MaxLen=m.PayloadBuffSize;

    int TxLength=MQTT_ConnectMsg(&TxBuffer,&PayloadBuffer,
                                 m.connectinfo.flags,
                                 m.connectinfo.keepalive,
                                 (const int8_t *)m.connectinfo.clientid,
                                 (const int8_t *)m.connectinfo.will_topic,
                                 (const int8_t *)m.connectinfo.username,
                                 (const int8_t *)m.connectinfo.password,
                                 (uint8_t *)m.connectinfo.will_payload,
                                 m.connectinfo.will_payload_length);

    app_debug_print("%s:Send Connect (%d Bytes) !!!\n\r",TAG,TxLength);

    if(send(socketfd,m.TxBuff,TxLength,0)<0)
    {
        m.connectstate.isconnected=false;
        app_debug_print("%s:Send Connect Failed!\n\r",TAG);
        iot_os_sleep(3000);
        return;
    }
    {
        //设置超时
        int timeout=30000;
        setsockopt(socketfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));

    }
    int RxLen=recv(socketfd,m.RxBuff,m.RxBuffSize,0);

    if(RxLen<=0)
    {
        m.connectstate.isconnected=false;
        app_debug_print("%s:Receive Connect Failed!\n\r",TAG);
        iot_os_sleep(3000);
        return;
    }
    {
        //设置超时
        int timeout=5;
        setsockopt(socketfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
    }

    {
        //处理接收的消息

        MQTT_HeadStruct head= {0};
        head.Data=(uint8_t *)m.PayloadBuff;
        uint8_t *Payload = NULL;
        uint32_t PayloadLen=0;
        uint32_t DealLen=0;
        Payload=MQTT_DecodeMsg(&head,m.PayloadBuffSize,&PayloadLen,(uint8_t *)m.RxBuff,RxLen,&DealLen);
        if(Payload==(void *)INVALID_HANDLE_VALUE)
        {
            m.connectstate.isconnected=false;
            app_debug_print("%s:Receive Data Predeal Failed!\n\r",TAG);
            iot_os_sleep(3000);
            return;
        }
        if(head.Cmd!= MQTT_CMD_CONNACK)
        {
            m.connectstate.isconnected=false;
            app_debug_print("%s:Unexcept CMD !\n\r",TAG);
            iot_os_sleep(3000);
            return;
        }

        if(Payload!=NULL)
        {
            if(Payload[1])
            {
                m.connectstate.isconnected=false;
                app_debug_print("%s:Connect Failed !\n\r",TAG);
                iot_os_sleep(3000);
                return;
            }
        }



    }

    m.connectstate.isconnected=true;

}
//成功连接后循环内回调函数(只能进行发送与接收操作),不可长时间阻塞
bool MQTT::appsocket_onloop(const struct __appsocket_cfg_t *cfg,int socketfd)//返回false重启socket
{
    MQTT &m=*(MQTT *)cfg->userptr;


    if(m.connectstate.isconnected)
    {
        return true;
    }
    else
    {
        return false;
    }
}
//关闭前回调
void MQTT::appsocket_before_close(const struct __appsocket_cfg_t *cfg,int socketfd)
{
    MQTT &m=*(MQTT *)cfg->userptr;
    m.connectstate.isconnected=false;
    m.connectstate.socketfd=-1;

}


//析构函数
MQTT::~MQTT()
{
    disconnect();
    iot_os_free(TxBuff);
    iot_os_free(RxBuff);
    iot_os_free(PayloadBuff);
}
