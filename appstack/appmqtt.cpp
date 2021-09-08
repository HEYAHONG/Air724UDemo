#include "appmqtt.hpp"
#include "config.h"
#include "debug.h"
#include "time.h"
#include "stdio.h"
#include "stdlib.h"
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
    if(clientid!=default_mqtt::clientid && clientid!=NULL)
    {
        iot_os_free(clientid);
    }
    if(username!=default_mqtt::username && username!=NULL)
    {
        iot_os_free(username);
    }
    if(password!=default_mqtt::password && password!=NULL)
    {
        iot_os_free(password);
    }
    if(will_topic!=default_mqtt::will_topic && will_topic!=NULL)
    {
        iot_os_free(will_topic);
    }
    will_payload_length=0;
    if(will_payload!=default_mqtt::will_payload && will_payload!=NULL)
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

MQTTConnectInfo::MQTTConnectInfo(const MQTTConnectInfo &other)
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
        if(other.will_payload_length!=0)
        {
            will_payload_length=other.will_payload_length;
            will_payload=iot_os_malloc(will_payload_length);
            memcpy(will_payload,other.will_payload,will_payload_length);
        }
        else
        {
            will_payload_length=other.will_payload_length;
            will_payload=default_mqtt::will_payload;
        }

    }
}

MQTTConnectInfo & MQTTConnectInfo::operator =(const MQTTConnectInfo &other)
{
    if(&other!=this)
    {
        set_clientid(other.clientid);
        set_flags(other.flags);
        set_keepalive(other.keepalive);
        set_username_and_password(other.username,other.password);
        set_willdata(other.will_topic,other.will_payload,other.will_payload_length);
    }
    return *this;
}

void MQTTConnectInfo::set_clientid(const char *_clientid,bool append_date)
{
    if(_clientid==NULL)
    {
        if(append_date)
        {
            {
                //clientid具有唯一性生成
                T_AMOPENAT_SYSTEM_DATETIME date= {0};
                iot_os_get_system_datetime(&date);
                char buff[128]= {0};
                sprintf(buff,"-%u-%u-%u-%u-%u-%u-%08X",date.nYear,date.nMonth,date.nDay,date.nHour,date.nMin,date.nSec,(unsigned int)iot_os_rand());
                size_t totallength=strlen(clientid)+strlen(buff)+1;
                char * new_clientid=(char *)iot_os_malloc(totallength);
                memset(new_clientid,0,totallength);
                strcat(new_clientid,clientid);
                strcat(new_clientid,buff);
                clientid=new_clientid;

            }
        }
    }
    else
    {
        char *old_clientid=clientid;
        char *new_clientid=(char *)iot_os_malloc(strlen(_clientid)+1);
        memset(new_clientid,0,strlen(_clientid)+1);
        strcpy(new_clientid,_clientid);
        clientid=new_clientid;
        if(old_clientid!=default_mqtt::clientid && old_clientid!=NULL)
        {
            iot_os_free(old_clientid);
        }

    }
}

void MQTTConnectInfo::set_flags(int _flags)
{
    flags=_flags;
}
void MQTTConnectInfo::set_keepalive(uint16_t _keepalive)
{
    keepalive=_keepalive;
    if(keepalive<6)
    {
        keepalive=6;
    }
}
bool MQTTConnectInfo::set_username_and_password(char *_username,char *_password)
{
    if(_username==NULL || _password ==NULL)
    {
        return false;
    }
    {
        char *old_username=username;
        char * new_username=(char *)iot_os_malloc(strlen(_username)+1);
        memset(new_username,0,strlen(_username)+1);
        memcpy(new_username,_username,strlen(_username));
        username=new_username;
        if(old_username!=default_mqtt::username && old_username!=NULL)
            iot_os_free(old_username);
    }

    {
        char *old_password=password;
        char * new_password=(char *)iot_os_malloc(strlen(_password)+1);
        memset(new_password,0,strlen(_password)+1);
        memcpy(new_password,_password,strlen(_password));
        password=new_password;
        if(old_password!=default_mqtt::password && old_password!=NULL)
            iot_os_free(old_password);
    }
    return true;
}
bool MQTTConnectInfo::set_willdata(char * _will_topic,void * _will_payload,size_t _will_payload_length)
{
    if(_will_payload==NULL || _will_topic ==NULL || _will_payload_length==0)
    {
        return false;
    }
    {
        char *old_will_topic=will_topic;
        char * new_will_topic=(char *)iot_os_malloc(strlen(_will_topic)+1);
        memset(new_will_topic,0,strlen(_will_topic)+1);
        memcpy(new_will_topic,_will_topic,strlen(_will_topic));
        will_topic=new_will_topic;
        if(old_will_topic!=default_mqtt::will_topic && old_will_topic!=NULL)
            iot_os_free(old_will_topic);
    }
    {
        char *old_will_payload=(char *)will_payload;
        char * new_will_payload=(char *)iot_os_malloc(_will_payload_length);
        memcpy(new_will_payload,_will_payload,_will_payload_length);
        will_payload=new_will_payload;
        will_payload_length=_will_payload_length;
        if(old_will_payload!=default_mqtt::will_payload && old_will_payload!=NULL)
            iot_os_free(old_will_payload);
    }

    return true;
}

MQTTSubscibeInfo::MQTTSubscibeInfo()
{
    topic=NULL;
    qos=0;
}

MQTTSubscibeInfo::MQTTSubscibeInfo(const MQTTSubscibeInfo &other)
{
    if(this!=&other)
    {
        topic=NULL;
        set_subscribe((char *)other.topic,other.qos);
    }
}
MQTTSubscibeInfo & MQTTSubscibeInfo::operator =(const MQTTSubscibeInfo &other)
{
    if(this!=&other)
    {
        set_subscribe((char *)other.topic,other.qos);
    }

    return *this;
}

MQTTSubscibeInfo::~MQTTSubscibeInfo()
{
    if(topic!=NULL)
    {
        iot_os_free(topic);
    }
}
bool MQTTSubscibeInfo::is_vailed()//是否有效
{
    if(topic!=NULL && strlen((char *)topic)>0)
    {
        return true;
    }
    return false;
}
bool MQTTSubscibeInfo::set_subscribe(char *_topic,uint8_t _qos)
{
    if(_topic==NULL || strlen((char *)_topic)==0)
    {
        return false;
    }
    {
        uint8_t *old_topic=topic;
        uint8_t *new_topic=(uint8_t *)iot_os_malloc(strlen(_topic)+1);
        memset(new_topic,0,strlen(_topic)+1);
        memcpy(new_topic,_topic,strlen(_topic));
        topic=new_topic;
        if(old_topic!=NULL)
        {
            iot_os_free(old_topic);
        }
    }
    qos=_qos;
    if(qos!=0 &&qos != MQTT_SUBSCRIBE_QOS2 && qos != MQTT_SUBSCRIBE_QOS1)
    {
        qos=0;
    }
    return true;
}

MQTTCallback::MQTTCallback()
{
    on_data=NULL;
    on_connect=NULL;
    on_disconnect=NULL;
}


MQTTPublishInfo::MQTTPublishInfo()
{
    qos=0;
    retain=0;
    topic=NULL;
    payload=NULL;
    payload_length=0;
}
MQTTPublishInfo::MQTTPublishInfo(const MQTTPublishInfo &other)
{
    qos=0;
    retain=0;
    topic=NULL;
    payload=NULL;
    payload_length=0;
    if(this!=&other)
    {
        set_publish(other.topic,other.payload,other.payload_length,other.qos,other.retain);
    }
}
MQTTPublishInfo &MQTTPublishInfo::operator =(const MQTTPublishInfo &other)
{
    if(this!=&other)
    {
        set_publish(other.topic,other.payload,other.payload_length,other.qos,other.retain);
    }
    return *this;
}
MQTTPublishInfo::~MQTTPublishInfo()
{
    if(topic!=NULL)
    {
        iot_os_free(topic);
    }
    if(payload!=NULL)
    {
        iot_os_free(payload);
    }
}

bool MQTTPublishInfo::is_vailed()//是否有效
{
    if(topic==NULL || strlen((char *)topic)==0)
    {
        return false;
    }
    return true;
}
void MQTTPublishInfo::set_publish(char *_topic,void *_payload,size_t _payload_length,uint8_t _qos,int _retain)
{
    if(_topic==NULL || strlen((char *)_topic)==0)
    {
        return;
    }
    qos=_qos;
    retain=_retain;
    {
        char *old_topic=topic;
        char *new_topic=(char *)iot_os_malloc(strlen(_topic)+1);
        memset(new_topic,0,strlen(_topic)+1);
        memcpy(new_topic,_topic,strlen(_topic));
        topic=new_topic;
        if(old_topic!=NULL)
        {
            iot_os_free(old_topic);
        }
    }
    {
        void *old_payload=payload;
        if(_payload!=NULL && _payload_length!=0)
        {
            payload=iot_os_malloc(_payload_length);
            memcpy(payload,_payload,_payload_length);
            payload_length=_payload_length;
        }
        else
        {
            payload=NULL;
            payload_length=0;
        }
        if(old_payload!=NULL)
        {
            iot_os_free(old_payload);
        }
    }

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

    return connect(appsocket_get_addr_by_ip(ip,port));

}



bool MQTT::connect(const struct openat_sockaddr_in addr)
{

    app_debug_print("%s:start connecting !!!\n\r",TAG);

    disconnect();

    appsocket_cfg_t cfg= {0};

    cfg.server_addr=addr;

    cfg.userptr=this;//传递this指针

    cfg.task_stack_size=RxBuffSize+2048;//足够大的栈空间用于执行回调函数

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
        connectstate.ispendingdisconnect=true;
        iot_os_sleep(100);
        appsocket_remove(connectstate.appsocketid);
        connectstate.socketfd=-1;
        connectstate.appsocketid=-1;
        connectstate.ispendingdisconnect=false;
    }
}

bool MQTT::get_is_connected()
{
    return connectstate.isconnected;
}

bool MQTT::subscribe(char *topic,uint8_t qos)
{
    if(!get_is_connected())
    {
        return false;
    }
    MQTTSubscibeInfo sub;
    sub.set_subscribe(topic,qos);

    if(sub.is_vailed())
    {

        subscribeinfo.lock.take();

        subscribeinfo.Queue.push(sub);

        subscribeinfo.lock.release();

        app_debug_print("%s:add subscribe to queue!!!\n\r",TAG);
    }

    return sub.is_vailed();
}

bool MQTT::unsubscribe(char *topic,uint8_t qos)
{
    if(!get_is_connected())
    {
        return false;
    }
    MQTTUnsubscribeInfo unsub;
    unsub.set_subscribe(topic,qos);

    if(unsub.is_vailed())
    {

        unsubscribeinfo.lock.take();

        unsubscribeinfo.Queue.push(unsub);

        unsubscribeinfo.lock.release();

        app_debug_print("%s:add unsubscribe to queue!!!\n\r",TAG);
    }

    return unsub.is_vailed();
}

bool MQTT::publish(char *_topic,void *_payload,size_t _payload_length,uint8_t _qos,int _retain)
{
    if(!get_is_connected())
    {
        return false;
    }
    MQTTPublishInfo pub;

    pub.set_publish(_topic,_payload,_payload_length,_qos,_retain);

    if(pub.is_vailed())
    {

        publishinfo.lock.take();

        publishinfo.Queue.push(pub);

        publishinfo.lock.release();

        app_debug_print("%s:add publish to queue!!!\n\r",TAG);
    }

    return pub.is_vailed();
}

void MQTT::set_callback(MQTTCallback cb)
{
    callback=cb;
}

//连接前回调函数
void MQTT::appsocket_before_connect(const struct __appsocket_cfg_t * cfg,int socket_fd)
{
    MQTT &m=*(MQTT *)cfg->userptr;
    m.connectstate.isconnected=false;
    m.connectstate.ispendingdisconnect=false;
    m.connectstate.socketfd=socket_fd;
    m.connectstate.mqttpackedid=0;
    if(strcmp((char *)default_mqtt::clientid,(char *)m.connectinfo.clientid)==0)
    {
        m.connectinfo.set_clientid(NULL,true);//使用cliendid
    }

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
        //启用TCP NODelay
        int nodelay=1;
        setsockopt(socketfd,0x06,OPENAT_TCP_NODELAY,&nodelay,sizeof(nodelay));
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

        m.connectstate.mqttpackedid=head.PackID;

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

    m.keepalivestate.last_tick=iot_os_get_system_tick();
    m.keepalivestate.is_send_req=false;
    m.keepalivestate.is_send_req_2=false;

    m.connectstate.isconnected=true;

    //调用连接回调
    if(m.callback.on_connect!=NULL)
    {
        m.callback.on_connect(m);
    }

}
//成功连接后循环内回调函数(只能进行发送与接收操作),不可长时间阻塞
bool MQTT::appsocket_onloop(const struct __appsocket_cfg_t *cfg,int socketfd)//返回false重启socket
{
    MQTT &m=*(MQTT *)cfg->userptr;

    bool Is_Send=false;//是否发送过(防止一次发送多个包)

    {
        //接收数据
        int RxLen=recv(socketfd,m.RxBuff,m.RxBuffSize,0);
        if(RxLen>0)
        {
            size_t TotalDealLen=0;
            while(TotalDealLen<(size_t)RxLen)
            {
                MQTT_HeadStruct head= {0};
                head.Data=(uint8_t *)m.PayloadBuff;
                uint8_t *Payload = NULL;
                uint32_t PayloadLen=0;
                uint32_t DealLen=0;
                Payload=MQTT_DecodeMsg(&head,m.PayloadBuffSize,&PayloadLen,(uint8_t *)(m.RxBuff+TotalDealLen),(RxLen-TotalDealLen),&DealLen);
                TotalDealLen+=DealLen;
                {
                    //默认一次就接收一个数据包，其余情况暂时忽略。若数据包很大则考虑调整socket的接收超时
                    if(Payload!=(uint8_t *)INVALID_HANDLE_VALUE)
                    {
                        //m.connectstate.mqttpackedid=head.PackID;
                        switch(head.Cmd)
                        {
                        case MQTT_CMD_PUBLISH:
                        {
                            {
                                //需要足够大的栈空间执行回调函数
                                if(m.callback.on_data!=NULL && head.DataLen>0)
                                {
                                    char topic[head.DataLen+1]= {0};
                                    memcpy(topic,head.Data,head.DataLen);
                                    int qos=0;
                                    int retain=0;
                                    if(head.Flag&MQTT_MSG_QOS1)
                                    {
                                        qos=1;
                                    }
                                    if(head.Flag&MQTT_MSG_QOS2)
                                    {
                                        qos=2;
                                    }
                                    if(head.Flag&MQTT_MSG_RETAIN)
                                    {
                                        retain=1;
                                    }

                                    m.callback.on_data(m,topic,head.DataLen,Payload,PayloadLen,qos,retain);
                                }
                            }
                            switch(head.Flag&MQTT_MSG_QOS_MASK)
                            {
                            case MQTT_MSG_QOS1:
                            {
                                Buffer_Struct TxBuff= {(uint8_t *)m.TxBuff,0,m.TxBuffSize};
                                int TxLen=MQTT_PublishCtrlMsg(&TxBuff,MQTT_CMD_PUBACK,head.PackID);
                                if(TxLen>0)
                                {
                                    send(socketfd,m.TxBuff,TxLen,0);
                                    Is_Send=true;
                                }
                            }
                            break;
                            case MQTT_MSG_QOS2:
                            {
                                Buffer_Struct TxBuff= {(uint8_t *)m.TxBuff,0,m.TxBuffSize};
                                int TxLen=MQTT_PublishCtrlMsg(&TxBuff,MQTT_CMD_PUBREL,head.PackID);
                                if(TxLen>0)
                                {
                                    send(socketfd,m.TxBuff,TxLen,0);
                                    Is_Send=true;
                                }
                            }
                            break;
                            default:
                                break;
                            }

                        }
                        break;
                        case MQTT_CMD_PUBREC:
                        {
                            Buffer_Struct TxBuff= {(uint8_t *)m.TxBuff,0,m.TxBuffSize};
                            int TxLen=MQTT_PublishCtrlMsg(&TxBuff,MQTT_CMD_PUBREL,head.PackID);
                            if(TxLen>0)
                            {
                                send(socketfd,m.TxBuff,TxLen,0);
                                Is_Send=true;
                            }

                        }
                        break;
                        case MQTT_CMD_PUBREL:
                        {
                            Buffer_Struct TxBuff= {(uint8_t *)m.TxBuff,0,m.TxBuffSize};
                            int TxLen=MQTT_PublishCtrlMsg(&TxBuff,MQTT_CMD_PUBCOMP,head.PackID);
                            if(TxLen>0)
                            {
                                send(socketfd,m.TxBuff,TxLen,0);
                                Is_Send=true;
                            }
                        }
                        break;
                        case MQTT_CMD_PINGRESP:
                        {
                            m.keepalivestate.last_tick=iot_os_get_system_tick();
                            m.keepalivestate.is_send_req=false;
                            m.keepalivestate.is_send_req_2=false;
                            app_debug_print("%s:receive pingresp!!!tick=%u\n\r",TAG,m.keepalivestate.last_tick);
                        }
                        break;

                        case MQTT_CMD_SUBACK:
                        {
                            app_debug_print("%s:subscribe ack !!! packageid=%u\n\r",TAG,head.PackID);
                        }
                        break;

                        case MQTT_CMD_UNSUBACK:
                        {
                            app_debug_print("%s:unsubscribe ack !!! packageid=%u\n\r",TAG,head.PackID);
                        }
                        break;

                        default:
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }

                }
            }
        }

    }

    if(!Is_Send)
    {
        //准备ping
        if(((iot_os_get_system_tick()-m.keepalivestate.last_tick)/(1000/ms_per_tick))>m.connectinfo.keepalive/2)
        {
            if(!m.keepalivestate.is_send_req)
            {
                //未发送ping
                Buffer_Struct TxBuff= {(uint8_t *)m.TxBuff,0,m.TxBuffSize};
                int TxLen=MQTT_SingleMsg(&TxBuff, MQTT_CMD_PINGREQ);;
                if(TxLen>0)
                {
                    app_debug_print("%s:send pingreq!!!\n\r",TAG);
                    send(socketfd,m.TxBuff,TxLen,0);
                    Is_Send=true;
                }
                m.keepalivestate.is_send_req=true;
            }
            else
            {
                if(((iot_os_get_system_tick()-m.keepalivestate.last_tick)/(1000/ms_per_tick))>m.connectinfo.keepalive*3/4)
                {
                    //再试一次
                    if(!m.keepalivestate.is_send_req_2)
                    {
                        m.keepalivestate.is_send_req_2=true;
                        m.keepalivestate.is_send_req=false;//重发
                    }

                }
                else
                {
                    //断开连接，重连
                    if(((iot_os_get_system_tick()-m.keepalivestate.last_tick)/(1000/ms_per_tick))>m.connectinfo.keepalive)
                    {
                        app_debug_print("%s:wait pingresp failed!!!\n\r",TAG);
                        return false;
                    }
                }
            }
        }

    }

    {
        //检查订阅
        if(!Is_Send)
        {

            if(m.subscribeinfo.Queue.size()>0)
            {
                MQTTSubscibeInfo info;
                m.subscribeinfo.lock.take();


                {
                    info=m.subscribeinfo.Queue.front();
                    m.subscribeinfo.Queue.pop();
                }

                m.subscribeinfo.lock.release();

                app_debug_print("%s:read subscribe from queue!!!\n\r",TAG);

                if(info.is_vailed())
                {
                    MQTT_SubscribeStruct sub[1];
                    sub[0].Char=info.topic;
                    sub[0].Qos=info.qos;
                    {
                        m.connectstate.mqttpackedid++;
                        Buffer_Struct TxBuff= {(uint8_t *)m.TxBuff,0,m.TxBuffSize};
                        Buffer_Struct PayloadBuff= {(uint8_t *)m.PayloadBuff,0,m.PayloadBuffSize};
                        int TxLen=MQTT_SubscribeMsg(&TxBuff,&PayloadBuff,m.connectstate.mqttpackedid,sub,1);
                        if(TxLen>0)
                        {
                            app_debug_print("%s:send subscribe !!! %d bytes,packageid=%u\n\r",TAG,TxLen,m.connectstate.mqttpackedid);
                            send(socketfd,m.TxBuff,TxLen,0);
                            Is_Send=true;
                        }
                    }
                }
            }


        }

    }

    {
        //检查取消订阅
        if(!Is_Send)
        {

            if(m.unsubscribeinfo.Queue.size()>0)
            {
                MQTTUnsubscribeInfo info;
                m.unsubscribeinfo.lock.take();


                {
                    info=m.unsubscribeinfo.Queue.front();
                    m.unsubscribeinfo.Queue.pop();
                }

                m.unsubscribeinfo.lock.release();

                app_debug_print("%s:read unsubscribe from queue!!!\n\r",TAG);

                if(info.is_vailed())
                {
                    MQTT_SubscribeStruct sub[1];
                    sub[0].Char=info.topic;
                    sub[0].Qos=info.qos;
                    {
                        m.connectstate.mqttpackedid++;
                        Buffer_Struct TxBuff= {(uint8_t *)m.TxBuff,0,m.TxBuffSize};
                        Buffer_Struct PayloadBuff= {(uint8_t *)m.PayloadBuff,0,m.PayloadBuffSize};
                        int TxLen=MQTT_UnSubscribeMsg(&TxBuff,&PayloadBuff,m.connectstate.mqttpackedid,sub,1);
                        if(TxLen>0)
                        {
                            app_debug_print("%s:send unsubscribe !!! %d bytes,packageid=%u\n\r",TAG,TxLen,m.connectstate.mqttpackedid);
                            send(socketfd,m.TxBuff,TxLen,0);
                            Is_Send=true;
                        }
                    }
                }
            }


        }

    }

    {
        //检查发布
        if(!Is_Send)
        {

            if(m.publishinfo.Queue.size()>0)
            {
                MQTTPublishInfo info;
                m.publishinfo.lock.take();


                {
                    info=m.publishinfo.Queue.front();
                    m.publishinfo.Queue.pop();
                }

                m.publishinfo.lock.release();

                app_debug_print("%s:read publish from queue!!!\n\r",TAG);

                if(info.is_vailed())
                {
                    {
                        m.connectstate.mqttpackedid++;
                        uint8_t flags=0;
                        if(info.retain)
                        {
                            flags|=MQTT_MSG_RETAIN;
                        }
                        if(info.qos==1)
                        {
                            flags|=MQTT_MSG_QOS1;
                        }
                        if(info.qos==2)
                        {
                            flags|=MQTT_MSG_QOS2;
                        }

                        Buffer_Struct TxBuff= {(uint8_t *)m.TxBuff,0,m.TxBuffSize};
                        int TxLen=MQTT_PublishMsg(&TxBuff,flags,m.connectstate.mqttpackedid,(const int8_t *)info.topic,(uint8_t *)info.payload,info.payload_length);
                        if(TxLen>0)
                        {
                            app_debug_print("%s:send publish !!! %d bytes,packageid=%u\n\r",TAG,TxLen,m.connectstate.mqttpackedid);
                            send(socketfd,m.TxBuff,TxLen,0);
                            Is_Send=true;
                        }
                    }
                }
            }


        }

    }



    {
        //检查是否断开连接
        if(!Is_Send)
        {
            if(m.connectstate.ispendingdisconnect)
            {
                {
                    //发送disconnect
                    Buffer_Struct TxBuff= {(uint8_t *)m.TxBuff,0,m.TxBuffSize};
                    int TxLen=MQTT_SingleMsg(&TxBuff,MQTT_CMD_DISCONNECT);
                    if(TxLen>0)
                    {
                        send(socketfd,m.TxBuff,TxLen,0);
                        Is_Send=true;
                    }
                }

                //等待任务被删除
                for(size_t i=0; i<60; i++)
                {
                    iot_os_sleep(1000);
                }
            }
        }
    }


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

    //调用连接丢失回调
    if(m.callback.on_disconnect!=NULL)
    {
        m.callback.on_disconnect(m);
    }

}


//析构函数
MQTT::~MQTT()
{
    disconnect();
    iot_os_free(TxBuff);
    iot_os_free(RxBuff);
    iot_os_free(PayloadBuff);
}
