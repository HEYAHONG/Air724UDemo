#include "config.h"
#include "appstack.hpp"
#include "appsocket.hpp"
#include "appmqtt.hpp"
#include "debug.h"

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

bool app_loop()
{
    return true;
}

void app_exit()
{

}
