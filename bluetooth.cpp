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

#ifdef CONFIG_BT_SUPPORT

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


bool bluetooth_advertising_enable(bool enable)
{
    U_OPENAT_BT_IOTCTL_PARAM para;
    int ret=0;
    {
        //打开广播
        memset(&para,0,sizeof(para));
        para.advEnable=(enable?1:0);
        ret=iot_ble_iotctl(0,BLE_SET_ADV_ENABLE,para);
    }
    return ret!=0;
}

bool bluetooth_set_advertising_parameter(bluetooth_advertising_parameter_t adv_para)
{
    U_OPENAT_BT_IOTCTL_PARAM para;
    int ret=0;
    {
        //设置广播参数
        memset(&para,0,sizeof(para));
        para.advparam=new bluetooth_advertising_parameter_t;
        memcpy(para.advparam,&adv_para,sizeof(adv_para));
        ret=iot_ble_iotctl(0,BLE_SET_ADV_PARAM,para);
        delete para.advparam;
    }
    return ret!=0;
}

bool bluetooth_set_advertising_data(uint8_t *data,size_t len)
{
    if(data==NULL || len > BLE_MAX_ADV_MUBER || len==0 )
    {
        return false;
    }

    U_OPENAT_BT_IOTCTL_PARAM para;
    int ret=0;
    {
        //设置广播包
        memset(&para,0,sizeof(para));
        para.advdata=new T_OPENAT_BLE_ADV_DATA;
        memset(para.advdata,0,sizeof(T_OPENAT_BLE_ADV_DATA));
        memcpy(para.advdata->data,data,len);
        para.advdata->len=len;
        ret=iot_ble_iotctl(0,BLE_SET_ADV_DATA,para);
        delete para.advdata;
    }
    return ret!=0;

}

bool bluetooth_set_advertising_scan_response(uint8_t *data,size_t len)
{
    if(data==NULL || len > BLE_MAX_ADV_MUBER || len==0 )
    {
        return false;
    }

    U_OPENAT_BT_IOTCTL_PARAM para;
    int ret=0;
    {
        //设置扫描回复包
        memset(&para,0,sizeof(para));
        para.advdata=new T_OPENAT_BLE_ADV_DATA;
        memset(para.advdata,0,sizeof(T_OPENAT_BLE_ADV_DATA));
        memcpy(para.advdata->data,data,len);
        para.advdata->len=len;
        ret=iot_ble_iotctl(0,BLE_SET_SCANRSP_DATA,para);
        delete para.advdata;
    }
    return ret!=0;
}

bool bluetooth_scan_enable(bool enable)
{
    U_OPENAT_BT_IOTCTL_PARAM para;
    int ret=0;
    {
        //扫描设置
        memset(&para,0,sizeof(para));
        para.advEnable=(enable?1:0);
        ret=iot_ble_iotctl(0,BLE_SET_SCAN_ENABLE,para);
    }
    return ret!=0;
}

bool bluetooth_set_scan_parameter(bluetooth_scan_parameter_t scan_para)
{
    U_OPENAT_BT_IOTCTL_PARAM para;
    int ret=0;
    {
        //设置广播参数
        memset(&para,0,sizeof(para));
        para.scanparam=new bluetooth_scan_parameter_t;
        memcpy(para.scanparam,&scan_para,sizeof(scan_para));
        ret=iot_ble_iotctl(0,BLE_SET_SCAN_PARAM,para);
        delete para.scanparam;
    }
    return ret!=0;
}

static std::string name;

const char * bluetooth_get_name()
{
    if(name.empty())
    {
        return "";
    }
    return name.c_str();
}

bool bluetooth_set_name(const char * _name)
{
    if(_name==NULL)
    {
        return false;
    }
    name=_name;
    U_OPENAT_BT_IOTCTL_PARAM para;
    {
        //设置广播名称
        memset(&para,0,sizeof(para));
        para.data=(uint8_t *)name.c_str();
        iot_ble_iotctl(0,BLE_SET_NAME,para);
        iot_ble_iotctl(0,BT_SET_NAME,para);
    }
    return true;

}

bool bluetooth_set_beacon(bluetooth_beacon_data_t data)
{
    U_OPENAT_BT_IOTCTL_PARAM para;
    int ret=0;
    {
        //设置广播名称
        memset(&para,0,sizeof(para));
        para.beacondata=new bluetooth_beacon_data_t;
        memcpy(para.beacondata,&data,sizeof(data));
        ret=iot_ble_iotctl(0,BLE_SET_BEACON_DATA,para);
        delete para.beacondata;
    }

    return ret!=0;
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
        /*
        某些数据dataPtr不为空，长度却为0
        */
        if(msg->dataPtr!=NULL)
        {
            switch(msg->id)
            {
            case OPENAT_BLE_SET_SCAN_REPORT:
            {
                msg->dataPtr=(uint8_t *)(void *)new bluetooth_ble_scan_report_info_t;
                memcpy(msg->dataPtr,result->dataPtr,sizeof(bluetooth_ble_scan_report_info_t));
                msg->len=sizeof(bluetooth_ble_scan_report_info_t);
            }
            break;
            default:
            {
                //默认不处理,如需处理,需要请使用new分配内存
                msg->dataPtr=NULL;
            }
            break;
            }
        }
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
                    {
                        {
                            //做一些初始化工作
                            if(!name.empty())
                            {
                                //设置广播信息
                                U_OPENAT_BT_IOTCTL_PARAM para;
                                {
                                    //设置广播名称
                                    memset(&para,0,sizeof(para));
                                    para.data=(uint8_t *)name.c_str();
                                    iot_ble_iotctl(0,BLE_SET_NAME,para);
                                    iot_ble_iotctl(0,BT_SET_NAME,para);
                                }
                            }
                        }
                        if(mode==BLUETOOTH_BLE_CENTRAL)
                        {
                            //打开扫描（目前Air724只能在中心设备模式下扫描）
                            bluetooth_scan_enable(true);
                        }
                        if(mode==BLUETOOTH_BLE_PERIPHERAL)
                        {
                            //打开广播（防止影响扫描，仅外设模式时自动打开）
                            bluetooth_advertising_enable(true);
                        }
                    }
                    break;
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
                    {

                    }
                    break;
                    case OPENAT_BLE_SET_SCAN_ENABLE:
                    {
                        app_debug_print("%s:start scan\r\n",TAG);
                    }
                    break;
                    case OPENAT_BLE_SET_SCAN_DISENABLE:
                    {
                        app_debug_print("%s:stop scan\r\n",TAG);
                    }
                    break;
                    case OPENAT_BLE_SET_SCAN_REPORT:
                    {
                        bluetooth_ble_scan_report_info_t *info= (bluetooth_ble_scan_report_info_t *)msg->dataPtr;
                        char addrstr[50]= {0};
                        sprintf(addrstr,"%02X:%02X:%02X:%02X:%02X:%02X",(int)info->bdAddress.addr[0],(int)info->bdAddress.addr[1],(int)info->bdAddress.addr[2],(int)info->bdAddress.addr[3],(int)info->bdAddress.addr[4],(int)info->bdAddress.addr[5]);
                        app_debug_print("%s: %s  rssi %d\r\n",TAG,addrstr,(int8_t)info->rssi);

                    }
                    break;
                    case OPENAT_BLE_SET_SCAN_FINISH:
                    {
                        app_debug_print("%s:scan finish\r\n",TAG);
                    }
                    break;
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

#endif // CONFIG_BT_SUPPORT

void bluetooth_init()
{

    /*
    if(iot_bt_open(BLE_SLAVE))
    {
        iot_bt_close();
    }
    else
    {
        //不支持蓝牙
        return;
    }
    */

#ifndef  CONFIG_BT_SUPPORT
    //不支持蓝牙
    return;
#else
    if(bluetooth_task_handle!=NULL)
    {
        //任务已启动
        return;
    }

    cb_list=std::make_shared<std::vector<std::function<bluetooth_cb>>>();
    mode=BLUETOOTH_OFF;
    name=CONFIG_CSDK_PRO;


    bluetooth_task_handle=iot_os_create_task(bluetooth_task, NULL, 4096, 24, OPENAT_OS_CREATE_DEFAULT, (char *)"bluetooth");
#endif
}
