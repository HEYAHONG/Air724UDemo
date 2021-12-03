#include "kconfig.h"
#include "appstack.hpp"
#include "appsocket.hpp"
#include "appmqtt.hpp"
#include "debug.h"
#include "stdlib.h"
#include "appmqtt.hpp"

static __unused const char * TAG="appstack";

static uint8_t auto_task_priority=CONFIG_APPSTACK_MIN_AUTO_TASK_PRIORITY;
uint8_t app_get_auto_task_priority()
{
    return auto_task_priority++;
}

extern "C"
{

}



void app_init()
{
    {
        //等待网络连接
        while(NETWORK_STATE_CONNECTED!=network_get_state())
        {
            iot_os_sleep(1000);
            app_debug_print("%s:wait for network!!!\n\r",TAG);
        }
    }

    //MQTT初始化
    MQTT_Init();


}

bool app_loop()
{


    return true;
}

void app_exit()
{

}

AppLock::~AppLock()
{
    iot_os_delete_semaphore(handle);
}
AppLock::AppLock()
{
    handle=iot_os_create_semaphore(1);
}
bool AppLock::take(uint32_t wait)
{
    return iot_os_wait_semaphore(handle,wait);
}
void AppLock::release()
{
    iot_os_release_semaphore(handle);
}
