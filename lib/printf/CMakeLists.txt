file(GLOB PRINTF_C_CPP_FILES  *.h *.c)
add_library(printf  ${PRINTF_C_CPP_FILES})
target_include_directories(printf PUBLIC ./)
