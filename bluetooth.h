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

#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef __cplusplus
#include <functional>
void bluetooth_add_callback(std::function<bluetooth_cb> cb);

#endif // __cplusplus

#endif // BLUETOOTH_H
