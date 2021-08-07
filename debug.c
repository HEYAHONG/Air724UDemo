#include "config.h"
#include "debug.h"

//初始化app_debug,默认使用UART2,921600
void app_debug_init()
{
    T_AMOPENAT_UART_PARAM uartCfg;
    memset(&uartCfg, 0, sizeof(T_AMOPENAT_UART_PARAM));
    uartCfg.baud = OPENAT_UART_BAUD_921600; //波特率
    uartCfg.dataBits = 8;   //数据位
    uartCfg.stopBits = 1; // 停止位
    uartCfg.parity = OPENAT_UART_NO_PARITY; // 无校验
    uartCfg.flowControl = OPENAT_UART_FLOWCONTROL_NONE; //无流控

    iot_uart_open(OPENAT_UART_2,&uartCfg);
}

//输出调试信息
void app_debug_print(const char * fmt,...)
{
    char *buff=iot_os_malloc(257);
    memset(buff,0,257);
    {
        va_list args;
        va_start(args, fmt);
        vsnprintf(buff, 256, fmt, args);
    }
    iot_uart_write(OPENAT_UART_2,(UINT8 *)buff,strlen(buff));
    iot_os_free(buff);
}
