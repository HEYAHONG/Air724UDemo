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
add_subdirectory(lib/SimpleMQTTGateWayStack/lib EXCLUDE_FROM_ALL)
target_link_libraries(${target} PUBLIC SMGS)

#添加源代码
file(GLOB  C_CPP_FILES
*.c
*.h
*.cpp
*.hpp
appstack/*.c
appstack/*.h
appstack/*.cpp
appstack/*.hpp
bluetooth/*.cpp
bluetooth/*.hpp
bluetooth/*.c
bluetooth/*.h
lib/MQTT/*.c
lib/MQTT/*.h
)
target_sources(${target} PUBLIC ${C_CPP_FILES})
add_definitions(" -DMQTTCLIENT_PLATFORM_HEADER=MQTTAir724UG.h -DMQTT_TASK ")
INCLUDE_DIRECTORIES(
.
appstack
lib/MQTT/
bluetooth/
)

if(CONFIG_BUILD_APP_MBEDTLS)
file(GLOB  SSL_C_CPP_FILES  lib/SSL/*.c lib/SSL/*.h)
target_sources(${target} PUBLIC ${SSL_C_CPP_FILES})
INCLUDE_DIRECTORIES(lib/SSL/ )
endif()

#添加ril
target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/components/ril/inc)
add_library(main_ril ${CMAKE_SOURCE_DIR}/components/ril/src/at_process.c ${CMAKE_SOURCE_DIR}/components/ril/src/at_tok.c  ${CMAKE_SOURCE_DIR}/components/ril/src/ril_pal.c )
target_link_libraries(${target} PRIVATE main_ril)
target_include_directories(main_ril  PRIVATE ${CMAKE_SOURCE_DIR}/components/include)
target_include_directories(main_ril  PRIVATE ${CMAKE_SOURCE_DIR}/components/newlib/include)
target_include_directories(main_ril  PRIVATE ${CMAKE_SOURCE_DIR}/components/openat_inc)
target_include_directories(main_ril  PRIVATE ${CMAKE_SOURCE_DIR}/api/include)
target_include_directories(main_ril  PRIVATE ${CMAKE_SOURCE_DIR}/components/ril/inc)

#添加RC文件
add_subdirectory(RC EXCLUDE_FROM_ALL)
target_link_libraries(${target} PRIVATE RC)

#添加jsoncpp库
add_subdirectory(lib/jsoncpp/ EXCLUDE_FROM_ALL)
target_link_libraries(${target} PRIVATE jsoncpp)

#添加protobuf-c库
add_subdirectory(lib/protobuf-c/ EXCLUDE_FROM_ALL)
target_link_libraries(${target} PRIVATE protobuf-c)

#添加protobuf-c库
add_subdirectory(lib/printf/ EXCLUDE_FROM_ALL)
target_link_libraries(${target} PRIVATE printf)

#启用SFFS支持
if(CONFIG_BUILD_SFFS)
find_program(SFFSCFGGEN NAMES sffscfggen sffscfggen.exe PATHS ./sffs/ REQUIRED)
find_program(DTOOLS dtools REQUIRED)
add_custom_target(SFFSIMG
                  COMMAND ${CMAKE_COMMAND} -E make_directory  ${out_hex_dir}
                  COMMAND ${CMAKE_COMMAND} -E make_directory  ${out_hex_dir}/oldpac
                  COMMAND ${SFFSCFGGEN} ${CMAKE_CURRENT_SOURCE_DIR}/sffs/sffs ${out_hex_dir}/sffs.json ${CONFIG_FS_SYS_FLASH_OFFSET}  ${CONFIG_FS_SYS_FLASH_SIZE}
                  COMMAND ${DTOOLS} fbdevgen -d v  ${out_hex_dir}/sffs.json ${out_hex_dir}/oldpac/sffs.img
)
add_dependencies(${target} SFFSIMG)
endif()
