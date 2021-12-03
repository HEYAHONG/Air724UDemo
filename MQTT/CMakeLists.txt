
#添加paho库
file(GLOB  PAHO_MQTTPACKET_C_FILES paho.mqtt.embedded-c/MQTTPacket/src/*.c)
list(APPEND PAHO_SRCS
${PAHO_MQTTPACKET_C_FILES}
)
add_library(PAHO_MQTT STATIC ${PAHO_SRCS})
target_include_directories(PAHO_MQTT
PUBLIC paho.mqtt.embedded-c/MQTTPacket/src/
PUBLIC ./
)

