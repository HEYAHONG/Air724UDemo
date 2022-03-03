#include "iot_debug.h"
#include "iot_os.h"
#include "stdbool.h"
#include "kconfig.h"
#include "debug.h"
#include "iot_flash.h"
#include "stdlib.h"
#include "stdio.h"
#include "appstack.hpp"
#include "network.h"
#include "iot_pmd.h"

HANDLE main_task_handle=NULL;

uint64_t ms_per_tick=5;

static  __unused  const char * TAG="main";

static void main_task(PVOID pParameter)
{


    app_debug_print("%s:%s",TAG,CONFIG_APP_ENTER_MESSAGE"\n\r");

    {
        uint64_t current_tick=iot_os_get_system_tick();
        app_debug_print("%s:current tick=%u\n\r",TAG,current_tick);
        iot_os_sleep(500);//延时500ms
        uint64_t current_tick_after_500ms=iot_os_get_system_tick();
        app_debug_print("%s:current tick=%u after 500ms\n\r",TAG,current_tick_after_500ms);
        ms_per_tick=(500/(current_tick_after_500ms-current_tick));
        app_debug_print("%s:ms_per_tick=%u\n\r",TAG,ms_per_tick);
    }


    app_init();


    {
        UINT32 totalmemory=0,freememory=0;

        iot_os_mem_used(&totalmemory,&freememory);

        freememory=(totalmemory-freememory);

        //打印剩余内存
        app_debug_print("%s:Total Memory:%uBytes,Free Memory:%uBytes\n\r",TAG,totalmemory,freememory);
    }

    {
        UINT32 addr=0,length=0;

        iot_flash_getaddr(&addr,&length);

        //打印剩余flash
        app_debug_print("%s:User Flash Addr:0x%08X,%uBytes\n\r",TAG,addr,length);

    }



    while(true)
    {
        iot_os_sleep(1);
        if(!app_loop())
        {
            break;
        }
    }


    iot_os_delete_task(main_task_handle);
}

int appimg_enter(void *param)
{

#if CONFIG_APP_DEBUG == 1
    iot_debug_set_fault_mode(OPENAT_FAULT_HANG);
    iot_os_set_trace_port(4);
#endif // CONFIG_APP_DEBUG

    iot_pmd_exit_deepsleep();

    app_debug_init();


#if CONFIG_NETWORK_START_ON_BOOT == 1
    network_init();
#endif // CONFIG_NETWORK_START_ON_BOOT


    main_task_handle = iot_os_create_task(main_task, NULL, 4096, 5, OPENAT_OS_CREATE_DEFAULT, "main");
    return 0;
}

void appimg_exit(void)
{
    app_exit();
    app_debug_print("%s:%s",TAG,CONFIG_APP_EXIT_MESSAGE"\n\r");
}
