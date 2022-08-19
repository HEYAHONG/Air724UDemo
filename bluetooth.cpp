#include <memory>
#include <vector>
#include <functional>
#include "bluetooth.h"
#include "debug.h"
#include "string.h"

static __unused const char * TAG="bluetooth";
static bool has_bluetooth=false; /**< 是否支持蓝牙 */

bool bluetooth_hasbluetooth()
{
    return has_bluetooth;
}

static HANDLE bluetooth_task_handle=NULL;

static std::shared_ptr<std::vector<std::function<bluetooth_cb>>> cb_list;

void bluetooth_add_callback(bluetooth_callback_t cb)
{
    bluetooth_add_callback(std::function<bluetooth_cb>(cb));
}

void bluetooth_add_callback(std::function<bluetooth_cb> cb)
{
    if(cb_list!=NULL &&cb !=NULL)
    {
        cb_list->push_back(cb);
    }
}

void bluetooth_callback(T_OPENAT_BLE_EVENT_PARAM *result)
{
    if(!has_bluetooth || bluetooth_task_handle==NULL)
    {
        //未初始化
        return;
    }

    bluetooth_callback_msg_t *msg=new bluetooth_callback_msg_t;
    (*msg)=(*result);
    msg->dataPtr=new uint8_t[msg->len];
    memcpy(msg->dataPtr,result->dataPtr,msg->len);
    iot_os_send_message(bluetooth_task_handle,msg);
}

static void bluetooth_task(PVOID pParameter)
{
    has_bluetooth=true;
    while(true)
    {
        if(iot_os_available_message(bluetooth_task_handle))
        {
            bluetooth_callback_msg_t *msg=NULL;
            iot_os_wait_message(bluetooth_task_handle,(PVOID *)&msg);
            if(msg!=NULL)
            {
                if(cb_list!=NULL)
                {
                    for(auto cb: *cb_list)
                    {
                        if(cb!=NULL)
                        {
                            bluetooth_callback_msg_t msg_temp=(*msg);
                            uint8_t data[msg->len];
                            msg_temp.dataPtr=data;
                            memcpy(data,msg->dataPtr,msg->len);

                            cb(msg_temp);
                        }
                    }
                }

                {
                    //删除消息
                    delete msg->dataPtr;
                    delete msg;
                    msg=NULL;
                }
            }
        }
        iot_os_sleep(10);
    }
}


void bluetooth_init()
{
    if(iot_bt_open(BLE_SLAVE))
    {
        iot_bt_close();
    }
    else
    {
        //不支持蓝牙
        return;
    }

    if(bluetooth_task_handle!=NULL)
    {
        //任务已启动
        return;
    }

    cb_list=std::make_shared<std::vector<std::function<bluetooth_cb>>>();

    bluetooth_task_handle=iot_os_create_task(bluetooth_task, NULL, 4096, 24, OPENAT_OS_CREATE_DEFAULT, (char *)"bluetooth");
}
