
#include "kconfig.h"
#include "appstack.hpp"
#include "appmqtt.hpp"
#include "debug.h"
#include "stdlib.h"
#include "appmqtt.hpp"
#include "network.h"
#include <chrono>
#include <string>
#include <iomanip>
#include <ctime>



static __unused const char * TAG="appstack";

static uint8_t auto_task_priority=CONFIG_APPSTACK_MIN_AUTO_TASK_PRIORITY;
uint8_t app_get_auto_task_priority()
{
    return auto_task_priority++;
}




void app_init()
{

    auto network_callback=[=](NetWork_State_t current_state,bool is_state_change,int8_t csq)->void
    {
        if(current_state!=NETWORK_STATE_CONNECTED)
        {
            app_debug_print("%s:network is not connected,csq=%u!\r\n",TAG,(uint32_t)csq);
            return;
        }
        else
        {
            if(is_state_change)
            {
                std::chrono::time_point<std::chrono::system_clock> tp=std::chrono::system_clock::now();
                time_t time=std::chrono::system_clock::to_time_t(tp);
                struct tm tmstruct={0};
                localtime_r(&time,&tmstruct);
                std::stringstream timestream;
                timestream<<std::put_time(&tmstruct,"%F %T GMT");
                app_debug_print("%s:network is  connected,csq=%u!(%s)\r\n",TAG,(uint32_t)csq,timestream.str().c_str());

            }
        }

        //下列代码可能会多次调用

        //MQTT初始化
        MQTT_Init();
    };

    network_add_callback(network_callback);

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
