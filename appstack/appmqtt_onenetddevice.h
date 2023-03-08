#ifndef APPMQTT_ONENETDDEVICE_H
#define APPMQTT_ONENETDDEVICE_H

#include "appmqtt.hpp"

#ifdef __cplusplus

/*
下列函数均用于回调函数,请不要手动调用它们。
*/

void MQTT_OneNETDevice_Init(MQTT_Cfg_t &cfg);

void MQTT_OneNETDevice_Connect(MQTT_Cfg_t &cfg);

void MQTT_OneNETDevice_DisConnect(MQTT_Cfg_t &cfg);

void MQTT_OneNETDevice_OnMessage(MQTT_Cfg_t &cfg,MQTT_Message_Ptr_t msg);

#endif // __cplusplus



#endif // APPMQTT_ONENETDDEVICE_H
