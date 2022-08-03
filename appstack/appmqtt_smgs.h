#ifndef APPMQTT_SMGS_H
#define APPMQTT_SMGS_H
#include "appmqtt.hpp"

#ifdef __cplusplus

void MQTT_SMGS_Init(MQTT_Cfg_t &cfg);

void MQTT_SMGS_Connect(MQTT_Cfg_t &cfg);

void MQTT_SMGS_DisConnect(MQTT_Cfg_t &cfg);

#endif // __cplusplus

#endif // APPMQTT_SMGS_H
