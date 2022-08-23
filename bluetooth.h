#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "iot_bluetooth.h"
#include "stdint.h"

typedef T_OPENAT_BLE_EVENT_PARAM bluetooth_callback_msg_t;

typedef void bluetooth_cb(bluetooth_callback_msg_t msg);
typedef bluetooth_cb * bluetooth_callback_t;

/** \brief 初始化函数,不可在任务中调用
 *
 *
 */

void bluetooth_init();


/** \brief 是否有蓝牙外设
 *
 * \return bool 若未初始化蓝牙或者不支持蓝牙则返回假,否则返回真
 *
 */
bool bluetooth_hasbluetooth();



/** \brief 增加回调函数
 *
 * \param cb bluetooth_callback_t 回调函数
 *
 */
void bluetooth_add_callback(bluetooth_callback_t cb);


typedef enum
{
    BLUETOOTH_BLE_PERIPHERAL=BLE_SLAVE, /**< 外设模式 */
    BLUETOOTH_BLE_CENTRAL=BLE_MASTER, /**< 中心设备模式 */
    BLUETOOTH_CLASS_BT=BT_CLASSIC, /**< 经典蓝牙模式 */
    BLUETOOTH_OFF,/**< 蓝牙未打开或不可用 */
} BLUETOOTH_MODE;

/** \brief 获取当前工作模式
 *
 * \return BLUETOOTH_MODE 工作模式
 *
 */
BLUETOOTH_MODE bluetooth_get_currentmode();

/** \brief 切换工作模式（打开或者关闭蓝牙）。
 *
 *
 * \param workmode BLUETOOTH_MODE 工作模式
 * \return bool 是否成功
 *
 */
bool bluetooth_switch_mode(BLUETOOTH_MODE workmode);


/** \brief 是否打开广播
 *
 * \param enable bool 是否启用
 * \return bool 是否成功
 *
 */
bool bluetooth_advertising_enable(bool enable);

typedef T_OPENAT_BLE_ADV_PARAM bluetooth_advertising_parameter_t;

/** \brief 设置广播参数
 *
 * \param adv_para bluetooth_advertising_parameter_t 广播参数
 * \return bool 是否成功
 *
 */
bool bluetooth_set_advertising_parameter(bluetooth_advertising_parameter_t adv_para);

/** \brief 设置广播包数据.
 * 注意:此操作可能覆盖默认的广播包,需要按照BLE广播数据包格式填写
 *
 * \param data uint8_t* 数据指针
 * \param len size_t 数据长度，不可大于BLE_MAX_ADV_MUBER
 * \return bool 是否成功
 *
 */
bool bluetooth_set_advertising_data(uint8_t *data,size_t len);

/** \brief 设置扫描回复数据.
 * 注意:需要按照BLE广播数据包格式填写
 *
 * \param data uint8_t* 数据指针
 * \param len size_t 数据长度，不可大于BLE_MAX_ADV_MUBER
 * \return bool 是否成功
 *
 */
bool bluetooth_set_advertising_scan_response(uint8_t *data,size_t len);

/** \brief 是否打开扫描
 *
 * \param enable bool 是否启用
 * \return bool 是否成功
 *
 */
bool bluetooth_scan_enable(bool enable);

typedef T_OPENAT_BLE_SCAN_PARAM bluetooth_scan_parameter_t;

/** \brief 设置扫描参数
 *
 * \param scan_para bluetooth_scan_parameter_t 设置扫描参数
 * \return bool 是否成功
 *
 */
bool bluetooth_set_scan_parameter(bluetooth_scan_parameter_t scan_para);

typedef struct BT_ADDRESS
{
    uint8 addr[6];
} BT_ADDRESS_t;

typedef struct _ble_scan_report_info
{
    UINT8 name_length;
    UINT8 name[BLE_MAX_ADV_MUBER+1];
    UINT8 addr_type;
    BT_ADDRESS_t bdAddress;
    UINT8 event_type;
    UINT8 data_length;
    UINT8 manu_data[BLE_MAX_ADV_MUBER+1];
    UINT8 manu_len;
    UINT8 raw_data[BLE_MAX_ADV_MUBER+1];
    UINT8 rssi;
} bluetooth_ble_scan_report_info_t;


/** \brief 获取蓝牙名称
 *
 * \return const char* 蓝牙名称
 *
 */
const char * bluetooth_get_name();



/** \brief 设置蓝牙名称
 *
 * \param name const char* 蓝牙名称
 * \return bool 是否成功
 *
 */
bool bluetooth_set_name(const char * name);


typedef T_OPENAT_BLE_BEACON_DATA bluetooth_beacon_data_t;

/** \brief 设置beacon
 *
 * \param data bluetooth_beacon_data_t beacon数据
 * \return bool 是否成功
 *
 */
bool bluetooth_set_beacon(bluetooth_beacon_data_t data);

#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef __cplusplus
#include <functional>
void bluetooth_add_callback(std::function<bluetooth_cb> cb);

#endif // __cplusplus

#endif // BLUETOOTH_H
