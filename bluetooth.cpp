#include <memory>
#include <vector>
#include <queue>
#include <functional>
#include "bluetooth.h"
#include "debug.h"
#include "string.h"
#include "kconfig.h"
#include "debug.h"

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

static BLUETOOTH_MODE mode=BLUETOOTH_OFF;

BLUETOOTH_MODE bluetooth_get_currentmode()
{
    if(bluetooth_hasbluetooth())
    {
        return mode;
    }
    else
    {
        if(mode!=BLUETOOTH_OFF)
        {
            iot_bt_close();
        }
        mode=BLUETOOTH_OFF;
        return mode;
    }
}
bool bluetooth_switch_mode(BLUETOOTH_MODE workmode)
{
    if(workmode==mode)
    {
        return true;
    }
    if(bluetooth_hasbluetooth())
    {
        if(workmode!=BLUETOOTH_OFF)
        {
            if(mode!=BLUETOOTH_OFF)
                iot_bt_close();
            if(iot_bt_open((E_OPENAT_BT_MODE)(int)workmode))
            {
                app_debug_print("%s:bluetooth open success!\r\n",TAG);
                mode=workmode;
                {
                    //做一些初始化工作
                    {
                        //设置广播信息
                        U_OPENAT_BT_IOTCTL_PARAM para;
                        {
                            //设置广播名称
                            memset(&para,0,sizeof(para));
                            std::string name=CONFIG_CSDK_PRO;
                            para.data=(uint8_t *)name.c_str();
                            iot_ble_iotctl(0,BLE_SET_NAME,para);
                        }
                        {
                            //打开广播
                            memset(&para,0,sizeof(para));
                            para.advEnable=1;
                            iot_ble_iotctl(0,BLE_SET_ADV_ENABLE,para);
                        }

                    }
                }

            }
            else
            {
                app_debug_print("%s:bluetooth open failed!\r\n",TAG);
                mode=BLUETOOTH_OFF;
            }
        }
        else
        {
            iot_bt_close();
            mode=BLUETOOTH_OFF;
        }
    }
    else
    {
        if(mode!=BLUETOOTH_OFF)
        {
            iot_bt_close();
        }
        mode=BLUETOOTH_OFF;
    }

    return workmode==mode;
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
    if(msg->len!=0)
    {
        msg->dataPtr=new uint8_t[msg->len];
        memcpy(msg->dataPtr,result->dataPtr,msg->len);
    }
    else
    {
        msg->dataPtr=NULL;
    }

    iot_os_send_message(bluetooth_task_handle,msg);

}

static void bluetooth_task(PVOID pParameter)
{
    has_bluetooth=true;
    while(true)
    {
        {
            bluetooth_callback_msg_t *msg=NULL;
            iot_os_wait_message(bluetooth_task_handle,(PVOID *)&msg);
            if(msg!=NULL)
            {
                {
                    //app_debug_print("%s:bluetooth event %d\r\n",TAG,(int)msg->id);
                    //处理事件
                    switch(msg->id)
                    {
                    case OPENAT_BT_ME_ON_CNF:
                    case OPENAT_BT_ME_OFF_CNF:
                    case OPENAT_BT_VISIBILE_CNF:
                    case OPENAT_BT_HIDDEN_CNF:
                    case OPENAT_BT_SET_LOCAL_NAME_RES:
                    case OPENAT_BT_SET_LOCAL_ADDR_RES:
                    case OPENAT_BT_INQ_DEV_NAME:
                    case OPENAT_BT_INQ_COMP_CNF:
                    case OPENAT_BT_INQUIRY_CANCEL:
                    case OPENAT_BT_DEV_PAIR_COMPLETE:
                    case OPENAT_BT_DELETE_DEVICE_RES:
                    case OPENAT_BT_DEV_PIN_REQ:
                    case OPENAT_BT_SSP_NUM_IND:
                    case OPENAT_BT_SPP_CONNECT_IND:
                    case OPENAT_BT_SPP_DISCONNECT_IND:
                    case OPENAT_BT_SPP_DATA_RECIEVE_IND:
                    case OPENAT_BT_SPP_DATA_SEND_IND:
                    case OPENAT_BT_HFP_CONNECT_RES:
                    case OPENAT_BT_HFP_DISCONNECT_RES:
                    case OPENAT_BT_REOPEN_IND:
                    case OPENAT_BT_REOPEN_ACTION_IND:
                    case OPENAT_BT_HFP_CALLSETUP_OUTGOING:
                    case OPENAT_BT_HFP_CALLSETUP_INCOMING:
                    case OPENAT_BT_HFP_CALLSETUP_NONE:
                    case OPENAT_BT_HFP_RING_INDICATION:
                    case OPENAT_BT_AVRCP_CONNECT_COMPLETE:
                    case OPENAT_BT_AVRCP_DISCONNECT_COMPLETE:
                    case OPENAT_BLE_SET_PUBLIC_ADDR:
                    case OPENAT_BLE_SET_RANDOM_ADDR:
                    case OPENAT_BLE_ADD_WHITE_LIST:
                    case OPENAT_BLE_REMOVE_WHITE_LIST:
                    case OPENAT_BLE_CLEAR_WHITE_LIST:
                    case OPENAT_BLE_CONNECT:
                    case OPENAT_BLE_DISCONNECT:
                    case OPENAT_BLE_UPDATA_CONNECT:
                    case OPENAT_BLE_SET_ADV_PARA:
                    case OPENAT_BLE_SET_ADV_DATA:
                    case OPENAT_BLE_SET_ADV_ENABLE:
                    case OPENAT_BLE_SET_ADV_SCAN_RSP:
                    case OPENAT_BLE_SET_SCAN_PARA:
                    case OPENAT_BLE_SET_SCAN_ENABLE:
                    case OPENAT_BLE_SET_SCAN_DISENABLE:
                    case OPENAT_BLE_SET_SCAN_REPORT:
                    case OPENAT_BLE_SET_SCAN_FINISH:
                    case OPENAT_BLE_CONNECT_IND:
                    case OPENAT_BLE_DISCONNECT_IND:
                    case OPENAT_BLE_FIND_CHARACTERISTIC_IND:
                    case OPENAT_BLE_FIND_SERVICE_IND:
                    case OPENAT_BLE_FIND_CHARACTERISTIC_UUID_IND:
                    case OPENAT_BLE_READ_VALUE:
                    case OPENAT_BLE_RECV_DATA:
                    default:
                        break;
                    }
                }



                //调用外部回调函数
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
                    if(msg->dataPtr!=NULL)
                        delete msg->dataPtr;
                    delete msg;
                    msg=NULL;
                }
            }
        }

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
    mode=BLUETOOTH_OFF;


    bluetooth_task_handle=iot_os_create_task(bluetooth_task, NULL, 4096, 24, OPENAT_OS_CREATE_DEFAULT, (char *)"bluetooth");
}
