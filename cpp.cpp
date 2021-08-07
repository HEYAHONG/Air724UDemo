#include "debug.h"
/*
���ļ���C++��ĳЩ�����������ء�
�������һЩȫ��֧�֡�
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
