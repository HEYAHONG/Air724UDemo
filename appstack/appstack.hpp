#ifndef APPSTACK_HPP_INCLUDED
#define APPSTACK_HPP_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
#include "stdlib.h"
#include "stdint.h"

void app_init();

bool app_loop();

void app_exit();

uint8_t app_get_auto_task_priority();

//每个tick多少毫秒
extern uint64_t ms_per_tick;

#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef __cplusplus

class AppLock
{
    void *handle;
public:
    AppLock();
    ~AppLock();
    AppLock(const AppLock &other)=delete;
    bool take(uint32_t wait=0);
    void release();

};

#endif // __cplusplus

#endif // APPSTACK_HPP_INCLUDED
