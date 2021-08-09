#ifndef APPSTACK_HPP_INCLUDED
#define APPSTACK_HPP_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
#include "stdlib.h"
#include "stdint.h"

void app_init();

bool app_loop();

void app_exit();

uint8_t app_get_auto_task_priority();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // APPSTACK_HPP_INCLUDED
