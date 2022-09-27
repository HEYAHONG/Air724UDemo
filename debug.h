#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
#include "stdio.h"

//初始化app_debug,默认使用UART2,921600
void app_debug_init();

#ifndef iot_debug_print
void iot_debug_print(char *fmt, ...);
#endif // iot_debug_print

#define app_debug_print(fmt,...) {printf(fmt,##__VA_ARGS__);iot_debug_print((char *)("[applog] " fmt),##__VA_ARGS__);}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DEBUG_H_INCLUDED
