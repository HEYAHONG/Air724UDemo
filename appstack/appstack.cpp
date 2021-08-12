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
//连接
static void mqtt_on_connect(class MQTT &client)
{
    client.subscribe((char *)"air724ug/json/cmd",0);
}
//丢失连接
static  void mqtt_on_disconnect(class MQTT &client)
{

}

//数据
static  void mqtt_on_data(class MQTT &client,char * topic,size_t topiclen,void *payload,size_t payloadlen,uint8_t qos,int retain)
{
    app_debug_print("%s:mqtt topic=%s,payload=%s(%u Bytes)\n\r",TAG,topic,payload,payloadlen);
    client.publish((char *)"air724ug/json/cmd/echo",payload,payloadlen);
}


void app_init()
{
    MQTTConnectInfo mqttinfo;
    MQTTCallback cb;
    cb.on_connect=mqtt_on_connect;
    cb.on_disconnect=mqtt_on_disconnect;
    cb.on_data=mqtt_on_data;
    ptr=new MQTT(mqttinfo,0,0,0);
    ptr->set_callback(cb);
    ptr->connect("116.85.50.218",1883);
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
