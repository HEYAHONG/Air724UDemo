#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "iot_debug.h"
#include "iot_os.h"
#include "stdarg.h"
#include "iot_uart.h"
#include "iot_os.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

//初始化app_debug,默认使用UART2,921600
void app_debug_init();

//输出调试信息
void app_debug_print(const char * fmt,...);

#ifdef __cplusplus
#endif // __cplusplus

#endif // DEBUG_H_INCLUDED
