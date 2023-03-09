#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

extern uint64_t ms_per_tick;

//获取imei
const char * get_imei();

size_t get_free_memory();


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MAIN_H_INCLUDED
