#include "debug.h"
/*
���ļ���C++��ĳЩ�����������ء�
�������һЩȫ��֧�֡�
C++ȫ�ֱ����Ĺ��켰������С��ʹ��(��������)
*/

extern "C"
{


    /*
    20210807
    ����CSDKͷ�ļ�δ��ȫ����ʹ��c++�ؼ�����Ϊ������ʶ������˲���ֱ�Ӱ���CSDKͷ�ļ�
    */

#include "config.h"

#include "stdint.h"
#include "stdlib.h"

    /**�ڴ�����ӿ�malloc
    *@param		nSize:		 ������ڴ��С
    *@return	void *:       �ڴ�ָ��
    **/
    extern void * iot_os_malloc(size_t  nSize);


    /**�ڴ��ͷŽӿ�
    *@param		pMemory:	     �ڴ�ָ�룬malloc�ӿڷ���ֵ
    **/
    extern void iot_os_free(void * pMemory);

}


static  __unused const char * TAG="cpp";


//����operator new
void* operator new(size_t nsize)
{
    void *p=iot_os_malloc(nsize);

#if CONFIG_CPP_OP_NEW_DEBUG == 1
    app_debug_print("%s:operator new addr=%08X,length=%u\n\r",TAG,(uint32_t)p,nsize);
#endif // CONFIG_CPP_OP_NEW_DEBUG

    return p;
}


//����operator delete
void operator delete(void* pointee)
{
#if CONFIG_CPP_OP_DELETE_DEBUG == 1
    app_debug_print("%s:operator delete addr=%08X\n\r",TAG,(uint32_t)pointee);
#endif // CONFIG_CPP_OP_DELETE_DEBUG
    iot_os_free(pointee);
}

namespace std
{
// Helper for exception objects in <except>
void __throw_bad_exception(void)
{
    app_debug_print("%s: %s\n\r",TAG,__FUNCTION__);
}

// Helper for exception objects in <new>
void __throw_bad_alloc(void)
{
    app_debug_print("%s: %s\n\r",TAG,__FUNCTION__);
}

// Helper for exception objects in <typeinfo>
void __throw_bad_cast(void)
{
    app_debug_print("%s: %s\n\r",TAG,__FUNCTION__);
}

void __throw_bad_typeid(void)
{
    app_debug_print("%s: %s\n\r",TAG,__FUNCTION__);
}

// Helpers for exception objects in <stdexcept>
void __throw_logic_error(const char*)
{
    app_debug_print("%s: %s\n\r",TAG,__FUNCTION__);
}

void __throw_domain_error(const char*)
{
    app_debug_print("%s: %s\n\r",TAG,__FUNCTION__);
}

void __throw_invalid_argument(const char*)
{
    app_debug_print("%s: %s\n\r",TAG,__FUNCTION__);
}

void __throw_length_error(const char*)
{
    app_debug_print("%s: %s\n\r",TAG,__FUNCTION__);
}

void __throw_out_of_range(const char*)
{
    app_debug_print("%s: %s\n\r",TAG,__FUNCTION__);
}

void __throw_out_of_range_fmt(const char*, ...)
{
    app_debug_print("%s: %s\n\r",TAG,__FUNCTION__);
}

void __throw_runtime_error(const char*)
{
    app_debug_print("%s: %s\n\r",TAG,__FUNCTION__);
}

void __throw_range_error(const char*)
{
    app_debug_print("%s: %s\n\r",TAG,__FUNCTION__);
}

void __throw_overflow_error(const char*)
{
    app_debug_print("%s: %s\n\r",TAG,__FUNCTION__);
}

void __throw_underflow_error(const char*)
{
    app_debug_print("%s: %s\n\r",TAG,__FUNCTION__);
}

// Helpers for exception objects in <ios>
void __throw_ios_failure(const char*)
{
    app_debug_print("%s: %s\n\r",TAG,__FUNCTION__);
}

void __throw_system_error(int)
{
    app_debug_print("%s: %s\n\r",TAG,__FUNCTION__);
}

void __throw_future_error(int)
{
    app_debug_print("%s: %s\n\r",TAG,__FUNCTION__);
}

// Helpers for exception objects in <functional>
void __throw_bad_function_call()
{
    app_debug_print("%s: %s\n\r",TAG,__FUNCTION__);
}


}

//һ�㲻���õ��˺���
extern "C" void __libc_fini_array()
{
    //ɶҲ����
}
//һ�㲻���õ��˱���
extern "C"
{
    void * __dso_handle=NULL;
}
