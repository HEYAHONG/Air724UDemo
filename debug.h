#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
#include "stdio.h"
#include "printf.h"

#ifndef iot_debug_print
void iot_debug_print(char *fmt, ...);
#endif // iot_debug_print

#include "printf.h"
void debug_port_lock();
void debug_port_out(char character, void* arg);
void debug_port_unlock();

#define app_debug_print(fmt,...) {debug_port_lock();fctprintf(debug_port_out,NULL,fmt,##__VA_ARGS__);debug_port_unlock();iot_debug_print((char *)("[applog] " fmt),##__VA_ARGS__);}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DEBUG_H_INCLUDED
