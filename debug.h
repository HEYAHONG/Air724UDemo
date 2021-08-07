#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus



//初始化app_debug,默认使用UART2,921600
void app_debug_init();

//输出调试信息
void app_debug_print(const char * fmt,...);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DEBUG_H_INCLUDED
