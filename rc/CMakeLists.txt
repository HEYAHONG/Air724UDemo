


find_program(FSGEN fsgen PATHS . REQUIRED)
add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/RC_fs.c
                     COMMAND ${FSGEN}
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(RC STATIC RC.h RC.c ${CMAKE_CURRENT_SOURCE_DIR}/RC_fs.c)
target_include_directories(RC PUBLIC .)
