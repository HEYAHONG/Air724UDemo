#include "debug.h"
/*
此文件对C++的某些函数进行重载。
或者添加一些全局支持。
*/

extern "C"
{


    /*
    20210807
    由于CSDK头文件未完全避免使用c++关键字作为变量标识符，因此不能直接包含CSDK头文件
    */

#include "config.h"

#include "stdint.h"
#include "stdlib.h"

    /**内存申请接口malloc
    *@param		nSize:		 申请的内存大小
    *@return	void *:       内存指针
    **/
    extern void * iot_os_malloc(size_t  nSize);


    /**内存释放接口
    *@param		pMemory:	     内存指针，malloc接口返回值
    **/
    extern void iot_os_free(void * pMemory);

}


static  __unused const char * TAG="cpp";


//重载operator new
void* operator new(size_t nsize)
{
    void *p=iot_os_malloc(nsize);

 #if CONFIG_CPP_OP_NEW_DEBUG == 1
    app_debug_print("%s:operator new addr=%08X,length=%u\n\r",TAG,(uint32_t)p,nsize);
 #endif // CONFIG_CPP_OP_NEW_DEBUG

    return p;
}

//重载operator delete
void operator delete(void* pointee)
{
#if CONFIG_CPP_OP_DELETE_DEBUG == 1
    app_debug_print("%s:operator delete addr=%08X\n\r",TAG,(uint32_t)pointee);
#endif // CONFIG_CPP_OP_DELETE_DEBUG
    iot_os_free(pointee);
}
