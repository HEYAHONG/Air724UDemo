
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
    MQTTConnectInfo(MQTTConnectInfo &other);

    ~MQTTConnectInfo();

    void set_flags(int _flags);
    void set_keepalive(uint16_t _keepalive);
    bool set_username_and_password(char *_username,char *_password);
    bool set_willdata(char * _will_topic,void * _will_payload,size_t _will_payload_length);


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
    } connectstate;

    struct
    {
        uint64_t last_tick;
        bool is_send_req;
        bool is_send_req_2;
    } keepalivestate;
public:
    MQTT(MQTTConnectInfo & _connectinfo,size_t MaxTxBuffSize,size_t MaxRxBuffSize,size_t MaxPayloadBuffSize);
    ~MQTT();
    MQTT(const MQTT &other)=delete;//禁止复制

    bool connect(const char *ip,uint16_t port);

    void disconnect();

    bool get_is_connected();

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
