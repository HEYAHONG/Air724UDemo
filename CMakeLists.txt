set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ ")





#genconfig,使用Kconfig生成config.h
find_program(GENCONFIG genconfig)
find_program(PYTHON python)

if (NOT DEFINED GENCONFIG)
    message(FATAL_ERROR "Can not find genconfig")
endif()

#生成config.h
add_custom_command(OUTPUT config.h
                    COMMAND ${PYTHON} -m genconfig --header-path config.h
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    )


#menuconfig
find_program(MENUCONFIG menuconfig)

if (NOT DEFINED MENUCONFIG)
    message(FATAL_ERROR "Can not find menuconfig")
endif()

#添加menuconfig(若使用ninja，则可以使用ninja menuconfig配置Kconfig)
add_custom_target(menuconfig ${PYTHON} -m menuconfig
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                  USES_TERMINAL
                  )

add_definitions(" -DUSE_KCONFIG=1 ")

if(CONFIG_APPIMG_LOAD_FLASH)
add_appimg(${target} ${flash_ldscript} config.h)
elseif(CONFIG_APPIMG_LOAD_FILE)
add_appimg(${target} ${file_ldscript}  config.h)
endif()

#添加库需要在add_appimg后

#添加PAHO MQTT库
add_subdirectory(lib/MQTT)
target_link_libraries(${target} PRIVATE PAHO_MQTT)

#添加源代码
file(GLOB  C_CPP_FILES *.c *.cpp appstack/*.c appstack/*.cpp lib/MQTT/*.c)
target_sources(${target} PUBLIC ${C_CPP_FILES})
add_definitions(" -DMQTTCLIENT_PLATFORM_HEADER=MQTTAir724UG.h -DMQTT_TASK ")
INCLUDE_DIRECTORIES(. appstack)
