#ifndef APPSTACK_HPP_INCLUDED
#define APPSTACK_HPP_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
#include "stdlib.h"

void app_init();

bool app_loop();

void app_exit();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // APPSTACK_HPP_INCLUDED
