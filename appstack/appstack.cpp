#include "config.h"
#include "appstack.hpp"
#include "appsocket.hpp"
#include "appmqtt.hpp"
#include "debug.h"
#include "stdlib.h"

static __unused const char * TAG="appstack";

static uint8_t auto_task_priority=CONFIG_APPSTACK_MIN_AUTO_TASK_PRIORITY;
uint8_t app_get_auto_task_priority()
{
    return auto_task_priority++;
}

extern "C"
{

}

MQTT *ptr=NULL;

void app_init()
{
    MQTTConnectInfo mqttinfo;
    ptr=new MQTT(mqttinfo,0,0,0);
    ptr->connect("116.85.50.218",1883);
}

static bool issubscribe=false;
bool app_loop()
{

    {

        if(!issubscribe)
        {
            if(ptr->get_is_connected())
            {
                if(ptr->subscribe((char *)"air724ug/json/cmd",0))
                {
                    issubscribe=true;
                }
            }
        }
    }

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
