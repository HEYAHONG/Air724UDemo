#include "network.h"
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_network.h"
#include "iot_socket.h"
#include "stdlib.h"
#include "stdio.h"
#include "debug.h"
#include "config.h"
#include "stdbool.h"


static __unused const char * TAG="network";

static NetWork_State_t net_state=NETWORK_STATE_NO_INIT;

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
    T_OPENAT_NETWORK_CONNECT networkparam=
    {
        .apn=CONFIG_NETWORK_APN_APN,
        .username=CONFIG_NETWORK_APN_USERNAME,
        .password=CONFIG_NETWORK_APN_PASSWORD
    };

    iot_network_connect(&networkparam);

}

static HANDLE network_task_handle=NULL;
static void network_task(PVOID pParameter)
{
    app_debug_print("%s:network init \n\r",TAG);


    while(true)
    {
        T_OPENAT_NETWORK_STATUS state= {0};
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
        iot_os_sleep(500);
    }

    iot_os_delete_task(network_task_handle);

}

void network_init()
{
    network_task_handle = iot_os_create_task(network_task, NULL, 1024, 2, OPENAT_OS_CREATE_DEFAULT, "network");
    //注册网络状态回调函数
    iot_network_set_cb(networkIndCallBack);
}
