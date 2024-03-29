﻿
extern "C"
{
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_network.h"
#include "iot_socket.h"
#include "stdlib.h"
#include "stdio.h"
#include "debug.h"
#include "kconfig.h"
#include "stdbool.h"
}
#include "string.h"
#include "network.h"


static __unused const char * TAG="network";

static NetWork_State_t net_state=NETWORK_STATE_NO_INIT;

static network_callback_t callback= {NULL,NULL};

static std::vector<std::function<network_loop_cpp_t>> callbackcpp;

//设置网络回调
void network_set_callback(network_callback_t cb)
{
    callback=cb;
}

void network_add_callback(std::function<network_loop_cpp_t> func)
{
    callbackcpp.push_back(func);
}

NetWork_State_t network_get_state()
{
    return net_state;
}

static __unused void networkIndCallBack(E_OPENAT_NETWORK_STATE state)
{
    app_debug_print("%s:network ind state %d\n\r",TAG,state);
    switch(state)
    {
    case OPENAT_NETWORK_READY:
    {

        net_state=NETWORK_STATE_READY;

        app_debug_print("%s:network ind state ready\n\r",TAG);

#if CONFIG_NETWORK_START_CONNECT_READY == 1
        network_start_connect();
#endif // CONFIG_NETWORK_START_CONNECT_READY
    }
    break;
    case OPENAT_NETWORK_LINKED:
    {
        net_state=NETWORK_STATE_CONNECTED;
        app_debug_print("%s:network ind state linked\n\r",TAG);
    }
    break;
    case OPENAT_NETWORK_DISCONNECT:
    {
        net_state=NETWORK_STATE_DISCONNECT;
        app_debug_print("%s:network ind state disconnected\n\r",TAG);
    }
    break;
    default:
        break;
    }
}

void network_start_connect()
{
    T_OPENAT_NETWORK_CONNECT networkparam= {0};

    strcat(networkparam.apn,CONFIG_NETWORK_APN_APN);
    strcat(networkparam.username,CONFIG_NETWORK_APN_USERNAME);
    strcat(networkparam.password,CONFIG_NETWORK_APN_PASSWORD);


    iot_network_connect(&networkparam);

}

static HANDLE network_task_handle=NULL;
static void network_task(PVOID pParameter)
{
    app_debug_print("%s:network init \n\r",TAG);

    iot_os_sleep(800);

    NetWork_State_t current_state=net_state;

    if(callback.init!=NULL)
    {
        callback.init();
    }

    while(true)
    {
        T_OPENAT_NETWORK_STATUS state;
        memset(&state,0,sizeof(state));
        if(iot_network_get_status(&state))
        {
            switch(net_state)
            {
            case NETWORK_STATE_NO_INIT:
            {
                if(state.simpresent==OPENAT_NETWORK_TRUE)
                {
                    app_debug_print("%s:sim detected \n\r",TAG);
                    net_state=NETWORK_STATE_HAS_SIM;
                }
                else
                {
                    app_debug_print("%s:sim not detected \n\r",TAG);
                    net_state=NETWORK_STATE_NO_SIM;
                }

            }
            break;
            case NETWORK_STATE_NO_SIM:
            {
                if(state.simpresent==OPENAT_NETWORK_TRUE)
                {
                    app_debug_print("%s:sim detected \n\r",TAG);
                    net_state=NETWORK_STATE_HAS_SIM;
                }
            }
            break;

            default:
                break;
            }
        }

        {
            bool is_change=(current_state!=net_state);
            if(is_change)
            {
                current_state=net_state;
            }

            if(callback.loop!=NULL)
            {
                callback.loop(current_state,is_change,state.csq);
            }

            for(auto callback:callbackcpp)
            {
                if(callback!=NULL)
                {
                    callback(current_state,is_change,state.csq);
                }
            }

        }

        iot_os_sleep(3000);
    }

    iot_os_delete_task(network_task_handle);

}

void network_init()
{
    callback.init=NULL;
    callback.loop=NULL;
    network_task_handle = iot_os_create_task(network_task, NULL, 4096, 2, OPENAT_OS_CREATE_DEFAULT, (char *)"network");
    //注册网络状态回调函数
    iot_network_set_cb(networkIndCallBack);
}


