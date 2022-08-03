
#ifndef APPMQTT_HPP_INCLUDED
#define APPMQTT_HPP_INCLUDED

#include "stdint.h"
#include "stdlib.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "iot_os.h"

#ifdef __cplusplus
};
#endif // __cplusplus

#ifdef __cplusplus
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <functional>
#include <memory>

typedef struct
{
    std::string host;//主机名
    uint16_t port; //端口
    int keepalive; //保活时间(本代码中不可为0，默认120)
    bool cleansession;//是否开启干净会话
    std::string clientid;
    struct _auth
    {
        std::string username; //用户名
        std::string password;//密码
    } auth;
    struct _will
    {
        std::string will_topic;//遗嘱主题
        std::string will_payload;//遗嘱负载数据
        uint8_t will_qos;//遗嘱qos
        bool will_retain;//遗嘱是否为保留消息
    } will;

} MQTT_Cfg_t;

typedef struct
{
    std::string topic;
    std::string payload;
    uint8_t qos;
    bool retain;
} MQTT_Message_t;

typedef std::shared_ptr<MQTT_Message_t> MQTT_Message_Ptr_t;

/*
初始化MQTT
*/
void MQTT_Init();

typedef struct
{
    std::function<void(MQTT_Cfg_t &)> init;
    std::function<void(MQTT_Cfg_t &)> connect;
    std::function<void(MQTT_Cfg_t &)> disconnect;
} MQTT_Callback_t;

/*
设置回调函数
*/
void MQTT_Set_Callback(MQTT_Callback_t cb);


#endif // __cplusplus

#endif // APPMQTT_HPP_INCLUDED
