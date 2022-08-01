
extern "C"
{
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
#include "iot_fs.h"
#include "at_process.h"
#include "at_tok.h"
#include "RC.h"
}

HANDLE main_task_handle=NULL;

uint64_t ms_per_tick=5;

static  __unused  const char * TAG="main";

static char imei[32]= {0};//存储IMEI
static bool gsmGetIMEI(char* imeiOut)
{
    int err;
    ATResponse *p_response = NULL;
    char* line = NULL;
    //UINT8 index = 0;
    bool result = FALSE;
    if(!imeiOut)
    {
        return result;
    }

    err = at_send_command_numeric("AT+GSN", &p_response);
    if (err < 0 || p_response->success==0)
    {
        result = FALSE;
        goto end;
    }

    line = p_response->p_intermediates->line;

    if(line!=NULL)
    {
        strcpy(imeiOut,line);
    }
    result = TRUE;
end:
    at_response_free(p_response);
    return result;

}
extern "C" const char * get_imei()
{
    if(strlen(imei)!=0)
    {
        return imei;
    }
    if(gsmGetIMEI(imei))
    {
        return imei;
    }

    return NULL;
}


static void main_task(PVOID pParameter)
{

    {
        char * banner=(char *)RCGetHandle("banner");
        app_debug_print("%s:\n%s",TAG,banner);
    }

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

    {
        //获取IMEI
        while(get_imei()==NULL)
        {
            iot_os_sleep(2000);//延时2000ms
            app_debug_print("%s:wait for imei\n\r",TAG);
        }
        app_debug_print("%s:imei:%s\n\r",TAG,get_imei());
    }

    {
        UINT32 addr=0,length=0;

        iot_flash_getaddr(&addr,&length);

        //打印剩余flash
        app_debug_print("%s:User Flash Addr:0x%08X,%uBytes\n\r",TAG,addr,length);

#if CONFIG_APP_AUTOMOUNT_FLASH == 1
        {
            //将用户空间用于挂载文件系统,使用ios_fs.h中的接口访问
            T_AMOPENAT_USER_FSMOUNT mount= {0};
            mount.clkDiv=2;
            mount.exFlash=E_AMOPENAT_FLASH_INTERNAL;
            mount.offset=addr;
            mount.size=length;
            mount.path=(char *)CONFIG_APP_AUTOMOUNT_FLASH_PATH;

            bool is_mount_success=false;
            if(!iot_fs_mount(&mount))
            {
                iot_fs_format(&mount);
                if(iot_fs_mount(&mount))
                {
                    is_mount_success=true;
                }
            }
            else
            {
                is_mount_success=true;
            }
            if(is_mount_success)
            {
                app_debug_print("%s:mount internal flash on %s success\n\r",TAG,CONFIG_APP_AUTOMOUNT_FLASH_PATH);
                T_AMOPENAT_FILE_INFO info= {0};
                if(0<=iot_fs_get_fs_info((char *)CONFIG_APP_AUTOMOUNT_FLASH_PATH,&info))
                {
                    app_debug_print("%s: filesystem %s total %llu Bytes used %llu Bytes\n\r",TAG,CONFIG_APP_AUTOMOUNT_FLASH_PATH,info.totalSize,info.usedSize);
                }
            }
            else
            {
                app_debug_print("%s:mount internal flash failed\n\r",TAG);
            }
        }
#endif // CONFIG_APP_AUTOMOUNT_FLASH
        {
            T_AMOPENAT_FILE_INFO info= {0};
            if(0<=iot_fs_get_fs_info((char *)"/",&info))
            {
                app_debug_print("%s: filesystem %s total %llu Bytes used %llu Bytes\n\r",TAG,"/",info.totalSize,info.usedSize);
            }
        }

    }


    app_init();


    {
        UINT32 totalmemory=0,freememory=0;

        iot_os_mem_used(&totalmemory,&freememory);

        freememory=(totalmemory-freememory);

        //打印剩余内存
        app_debug_print("%s:Total Memory:%uBytes,Free Memory:%uBytes\n\r",TAG,totalmemory,freememory);
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

extern "C" int appimg_enter(void *param)
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


    main_task_handle = iot_os_create_task(main_task, NULL, 4096, 5, OPENAT_OS_CREATE_DEFAULT, (char *)"main");
    return 0;
}

extern "C" void appimg_exit(void)
{
    app_exit();
    app_debug_print("%s:%s",TAG,CONFIG_APP_EXIT_MESSAGE"\n\r");
}
