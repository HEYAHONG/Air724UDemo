#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "iot_bluetooth.h"

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
 *  似乎CSDK不支持多次打开或者关闭蓝牙，不能多次调用此函数
 *
 * \param workmode BLUETOOTH_MODE 工作模式
 * \return bool 是否成功
 *
 */
bool bluetooth_switch_mode(BLUETOOTH_MODE workmode);

#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef __cplusplus
#include <functional>
void bluetooth_add_callback(std::function<bluetooth_cb> cb);

#endif // __cplusplus

#endif // BLUETOOTH_H
