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

#添加SMGS库
add_subdirectory(lib/SimpleMQTTGateWayStack/lib)
target_link_libraries(${target} PUBLIC SMGS)

#添加源代码
file(GLOB  C_CPP_FILES *.c *.h *.cpp *.hpp appstack/*.c appstack/*.h  appstack/*.cpp appstack/*.hpp lib/MQTT/*.c lib/MQTT/*.h)
target_sources(${target} PUBLIC ${C_CPP_FILES})
add_definitions(" -DMQTTCLIENT_PLATFORM_HEADER=MQTTAir724UG.h -DMQTT_TASK ")
INCLUDE_DIRECTORIES(. appstack lib/MQTT/)

#添加ril
target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/components/ril/inc)
add_library(main_ril ${CMAKE_SOURCE_DIR}/components/ril/src/at_process.c ${CMAKE_SOURCE_DIR}/components/ril/src/at_tok.c  ${CMAKE_SOURCE_DIR}/components/ril/src/ril_pal.c )
target_link_libraries(${target} PRIVATE main_ril)
target_include_directories(main_ril  PRIVATE ${CMAKE_SOURCE_DIR}/components/include)
target_include_directories(main_ril  PRIVATE ${CMAKE_SOURCE_DIR}/components/newlib/include)
target_include_directories(main_ril  PRIVATE ${CMAKE_SOURCE_DIR}/components/openat_inc)
target_include_directories(main_ril  PRIVATE ${CMAKE_SOURCE_DIR}/api/include)
target_include_directories(main_ril  PRIVATE ${CMAKE_SOURCE_DIR}/components/ril/inc)
