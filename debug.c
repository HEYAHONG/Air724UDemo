#include "kconfig.h"
#include "debug.h"
#include "iot_debug.h"
#include "iot_os.h"
#include "stdarg.h"
#include "iot_uart.h"
#include "iot_os.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "printf.h"

static HANDLE lock=NULL;

//初始化debug_port,默认使用UART2,921600
void debug_port_init()
{
    T_AMOPENAT_UART_PARAM uartCfg;
    memset(&uartCfg, 0, sizeof(T_AMOPENAT_UART_PARAM));
    uartCfg.baud = OPENAT_UART_BAUD_921600; //波特率
    uartCfg.dataBits = 8;   //数据位
    uartCfg.stopBits = 1; // 停止位
    uartCfg.parity = OPENAT_UART_NO_PARITY; // 无校验
    uartCfg.flowControl = OPENAT_UART_FLOWCONTROL_NONE; //无流控

    iot_uart_open(OPENAT_UART_2,&uartCfg);

    lock=iot_os_create_semaphore(1);
}


void debug_port_lock()
{
    if(lock == NULL)
    {
        debug_port_init();
    }

    iot_os_wait_semaphore(lock,0);
}

void debug_port_out(char character, void* arg)
{
    (void)arg;
    iot_uart_write(OPENAT_UART_2,(UINT8 *)&character,1);
}

void debug_port_unlock()
{
    iot_os_release_semaphore(lock);
}

int write_tty(char *ptr,int len)
{
    if(lock==NULL)
    {
        return 0;
    }

    iot_os_wait_semaphore(lock,0);

    iot_uart_write(OPENAT_UART_2,(UINT8 *)ptr,len);

    iot_os_release_semaphore(lock);

    return len;
}

int read_tty(char *ptr,int len)
{
    if(lock==NULL)
    {
        return 0;
    }

    iot_os_wait_semaphore(lock,0);

    len=iot_uart_read(OPENAT_UART_2,(UINT8 *)ptr,len,2000);

    iot_os_release_semaphore(lock);

    return len;
}
