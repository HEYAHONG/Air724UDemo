#include "iot_debug.h"
#include "iot_os.h"
#include "stdbool.h"
#include "config.h"
#include "debug.h"

HANDLE main_task_handle=NULL;

const char * TAG=__FILE__;

static void main_task(PVOID pParameter)
{


    app_debug_print(CONFIG_APP_ENTER_MESSAGE"\n\r");

    {
        UINT32 totalmemory=0,freememory=0;

        iot_os_mem_used(&totalmemory,&freememory);

        //打印剩余内存
        app_debug_print("Total Memory:%ubytes,Free Memory:%ubytes\n\r",totalmemory,freememory);
    }


    while(true)
    {
        iot_os_sleep(1);
    }


    iot_os_delete_task(main_task_handle);
}

int appimg_enter(void *param)
{

#if CONFIG_APP_DEBUG == 1
    iot_debug_set_fault_mode(OPENAT_FAULT_HANG);
#endif // CONFIG_APP_DEBUG

    app_debug_init();


    main_task_handle = iot_os_create_task(main_task, NULL, 1024, 1, OPENAT_OS_CREATE_DEFAULT, "main");
    return 0;
}

void appimg_exit(void)
{
    app_debug_print(CONFIG_APP_EXIT_MESSAGE"\n\r");
}
