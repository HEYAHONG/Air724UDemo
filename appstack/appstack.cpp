#include "config.h"
#include "appstack.hpp"
#include "vector"

static uint8_t auto_task_priority=CONFIG_APPSTACK_MIN_AUTO_TASK_PRIORITY;
uint8_t app_get_auto_task_priority()
{
    return auto_task_priority++;
}

void app_init()
{


}

bool app_loop()
{
    return true;
}

void app_exit()
{

}
