


find_program(FSGEN fsgen PATHS . REQUIRED)
file(GLOB_RECURSE FS_FILES fs/*)
add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/RC_fs.c
                     COMMAND ${FSGEN} ${CMAKE_CURRENT_SOURCE_DIR}/fs ${CMAKE_CURRENT_SOURCE_DIR}/RC_fs.c
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                     BYPRODUCTS ${FS_FILES}
)

add_library(RC STATIC RC.h RC.c ${CMAKE_CURRENT_SOURCE_DIR}/RC_fs.c)
target_include_directories(RC PUBLIC .)
