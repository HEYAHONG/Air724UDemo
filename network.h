#ifndef NETWORK_H_INCLUDED
#define NETWORK_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
#include "stdbool.h"
#include "stdint.h"
#include "kconfig.h"

typedef enum
{
    NETWORK_STATE_NO_INIT=0,
    NETWORK_STATE_NO_SIM,
    NETWORK_STATE_HAS_SIM,
    NETWORK_STATE_READY,
    NETWORK_STATE_CONNECTED,
    NETWORK_STATE_DISCONNECT,
} NetWork_State_t;

//初始化(不可在任务中调用)
void network_init();

void network_start_connect();

//获取当前状态
NetWork_State_t network_get_state();

typedef void (*network_init_t)();
typedef void (*network_loop_t)(NetWork_State_t current_state,bool is_state_change,int8_t csq);
typedef struct
{
    network_init_t init;
    network_loop_t loop;
} network_callback_t;

//设置网络回调
void network_set_callback(network_callback_t cb);

#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef __cplusplus

#include "functional"
#include "vector"
typedef void (network_loop_cpp_t)(NetWork_State_t current_state,bool is_state_change,int8_t csq);

//CPP回调函数,只在任务循环调用
void network_add_callback(std::function<network_loop_cpp_t> func);

#endif // __cplusplus

#endif // NETWORK_H_INCLUDED
