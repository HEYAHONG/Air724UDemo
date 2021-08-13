﻿
#ifndef APPMQTT_HPP_INCLUDED
#define APPMQTT_HPP_INCLUDED

#include "appsocket.hpp"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "iot_os.h"
extern uint64_t ms_per_tick;


#ifdef __cplusplus
};
#endif // __cplusplus

#ifdef __cplusplus
#include "queue"
#include "appstack.hpp"
class MQTT;

class MQTTConnectInfo
{
    friend class MQTT;
    uint16_t keepalive;

    int flags;

    //Clientid
    char * clientid;

    //账号及密码
    char *username;
    char *password;

    //遗嘱消息相关
    char *will_topic;
    void *will_payload;
    size_t will_payload_length;

public:
    MQTTConnectInfo();
    MQTTConnectInfo(const MQTTConnectInfo &other);
    MQTTConnectInfo & operator =(const MQTTConnectInfo &other);

    ~MQTTConnectInfo();

    void set_clientid(const char *_clientid,bool append_date=false);
    void set_flags(int _flags);
    void set_keepalive(uint16_t _keepalive);
    bool set_username_and_password(char *_username,char *_password);
    bool set_willdata(char * _will_topic,void * _will_payload,size_t _will_payload_length);


};

class MQTTSubscibeInfo
{
    friend class MQTT;
    uint8_t * topic;
    uint8_t     qos;
public:
    MQTTSubscibeInfo();
    MQTTSubscibeInfo(const MQTTSubscibeInfo &other);
    MQTTSubscibeInfo & operator =(const MQTTSubscibeInfo &other);
    ~MQTTSubscibeInfo();
    bool is_vailed();//是否有效
    bool set_subscribe(char *_topic,uint8_t _qos);
};

class MQTTCallback
{
public:
    MQTTCallback();
    //连接
    void (*on_connect)(class MQTT &client);
    //丢失连接
    void (*on_disconnect)(class MQTT &client);

    //数据
    void (*on_data)(class MQTT &client,char * topic,size_t topiclen,void *payload,size_t payloadlen,uint8_t qos,int retain);
};

class MQTTPublishInfo
{
    friend class MQTT;
    uint8_t qos;
    int retain;
    char *topic;
    void *payload;
    size_t payload_length;

public:
    MQTTPublishInfo();
    MQTTPublishInfo(const MQTTPublishInfo &other);
    MQTTPublishInfo &operator =(const MQTTPublishInfo &other);
    ~MQTTPublishInfo();

    bool is_vailed();//是否有效
    void set_publish(char *_topic,void *_payload,size_t _payload_length,uint8_t _qos,int _retain);
};

class MQTT
{
    MQTTConnectInfo connectinfo;
    char *TxBuff;
    size_t TxBuffSize;
    char *RxBuff;
    size_t RxBuffSize;
    char *PayloadBuff;
    size_t PayloadBuffSize;

    struct
    {
        int appsocketid;
        int socketfd;
        bool isconnected;
        bool ispendingdisconnect;
        uint16_t mqttpackedid;
    } connectstate;

    struct
    {
        uint64_t last_tick;
        bool is_send_req;
        bool is_send_req_2;
    } keepalivestate;

    struct
    {
        AppLock lock;
        std::queue<MQTTSubscibeInfo> Queue;
    } subscribeinfo;

    using MQTTUnsubscribeInfo=MQTTSubscibeInfo;
    struct
    {
        AppLock lock;
        std::queue<MQTTUnsubscribeInfo> Queue;
    } unsubscribeinfo;

    MQTTCallback callback;

    struct
    {
        AppLock lock;
        std::queue<MQTTPublishInfo> Queue;
    } publishinfo;

public:
    MQTT(MQTTConnectInfo & _connectinfo,size_t MaxTxBuffSize,size_t MaxRxBuffSize,size_t MaxPayloadBuffSize);
    ~MQTT();
    MQTT(const MQTT &other)=delete;//禁止复制

    bool connect(const char *ip,uint16_t port=1883);

    bool connect(const struct openat_sockaddr_in addr);

    void disconnect();

    bool get_is_connected();

    bool subscribe(char *topic,uint8_t qos=0);

    bool unsubscribe(char *topic,uint8_t qos=0);

    bool publish(char *_topic,void *_payload,size_t _payload_length,uint8_t _qos=0,int _retain=0);

    void set_callback(MQTTCallback cb);

    //appsocket相关回调
    //连接前回调函数
    static void appsocket_before_connect(const struct __appsocket_cfg_t * cfg,int socket_fd);
    //连接成功后回调函数
    static void appsocket_after_connect(const struct __appsocket_cfg_t *cfg,int socketfd);
    //成功连接后循环内回调函数(只能进行发送与接收操作),不可长时间阻塞
    static bool appsocket_onloop(const struct __appsocket_cfg_t *cfg,int socketfd);//返回false重启socket
    //关闭前回调
    static void appsocket_before_close(const struct __appsocket_cfg_t *cfg,int socketfd);

};


#endif // __cplusplus

#endif // APPMQTT_HPP_INCLUDED
