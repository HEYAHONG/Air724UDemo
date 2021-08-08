#ifndef NETWORK_H_INCLUDED
#define NETWORK_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

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


#ifdef __cplusplus
}
#endif // __cplusplus


#endif // NETWORK_H_INCLUDED
