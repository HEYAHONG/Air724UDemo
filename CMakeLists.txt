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

if(CONFIG_APPIMG_LOAD_FLASH)
add_appimg(${target} ${flash_ldscript} main.c debug.c config.h)
elseif(CONFIG_APPIMG_LOAD_FILE)
add_appimg(${target} ${file_ldscript} main.c debug.c config.h)
endif()
