#include "MQTTAir724UG.h"
#include "stdint.h"
#include "string.h"
#include "iot_socket.h"
#include "iot_os.h"
#include "errno.h"
#include "debug.h"
#if CONFIG_MQTT_SSL == 1
#include "APPSSL.h"
#endif // CONFIG_BUILD_APP_MBEDTLS

extern uint64_t ms_per_tick;

int ThreadStart(Thread* thread, void (*fn)(void*), void* arg)
{
    int rc = 0;


    return rc;
}


void MutexInit(Mutex* mutex)
{
    if(mutex->sem!=NULL)
    {
        /*
        尝试删除sem。仅针对appstack中Client的写法。
        */
        iot_os_delete_semaphore(mutex->sem);
    }
    mutex->sem =iot_os_create_semaphore(1);
}

int MutexLock(Mutex* mutex)
{
    return iot_os_wait_semaphore(mutex->sem,0);
}

int MutexUnlock(Mutex* mutex)
{
    return iot_os_release_semaphore(mutex->sem);
}


void TimerCountdownMS(Timer* timer, unsigned int timeout_ms)
{
    timer->timeout_ms=timeout_ms;
    timer->start_tick=iot_os_get_system_tick();

}


void TimerCountdown(Timer* timer, unsigned int timeout)
{
    TimerCountdownMS(timer, timeout * 1000);
}


int TimerLeftMS(Timer* timer)
{
    int ret=0;

    if(!TimerIsExpired(timer))
    {
        uint64_t tick=iot_os_get_system_tick()-timer->start_tick;
        ret=(timer->timeout_ms-tick*ms_per_tick);
    }

    return ret;
}


char TimerIsExpired(Timer* timer)
{
    return (iot_os_get_system_tick()-timer->start_tick)> (timer->timeout_ms/ms_per_tick);
}


void TimerInit(Timer* timer)
{
    memset(timer,0,sizeof(Timer));
}


int Air724UG_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
#if CONFIG_MQTT_SSL == 1
    {
        int retlen=0;
        app_mbedtls_read(n->SSL_Handle,buffer,len,timeout_ms,&retlen);
        if(retlen<0)
        {
            switch(retlen)
            {
            case MBEDTLS_ERR_SSL_WANT_READ:
            case MBEDTLS_ERR_SSL_WANT_WRITE:
            case MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS:
            case MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS:
                retlen=0;
            default:
                retlen=-1;
            }
        }
        return retlen;
    }
#else
    int recvLen = 0;

    int timetowaitms=timeout_ms;

    do
    {
        int rc = 0;

        setsockopt(n->my_socket,SOL_SOCKET,SO_RCVTIMEO, &timetowaitms, sizeof(timetowaitms));
        rc = recv(n->my_socket, buffer + recvLen, len - recvLen, 0);
        if (rc > 0)
            recvLen += rc;
        else if (rc < 0)
        {
            recvLen = rc;
            if(socket_errno(n->my_socket)!=ENOTCONN)
            {
                recvLen=0;
            }
            break;
        }
    }
    while (recvLen < len);

    return recvLen;
#endif // CONFIG_BUILD_APP_MBEDTLS
}


int  Air724UG_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{

#if CONFIG_MQTT_SSL == 1
    {
        int retlen=0;
        app_mbedtls_write(n->SSL_Handle,buffer,len,timeout_ms,&retlen);
        if(retlen<0)
        {
            switch(retlen)
            {
            case MBEDTLS_ERR_SSL_WANT_READ:
            case MBEDTLS_ERR_SSL_WANT_WRITE:
            case MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS:
            case MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS:
                retlen=0;
            default:
                retlen=-1;
            }
        }
        return retlen;
    }
#else
    int sentLen = 0;

    int timetowaitms=timeout_ms;

    do
    {
        int rc = 0;

        setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, &timetowaitms, sizeof(timetowaitms));
        rc = send(n->my_socket, buffer + sentLen, len - sentLen, 0);
        if (rc > 0)
            sentLen += rc;
        else if (rc < 0)
        {
            sentLen = rc;
            break;
        }
    }
    while (sentLen < len );

    return sentLen;
#endif // CONFIG_BUILD_APP_MBEDTLS
}


void  Air724UG_disconnect(Network* n)
{
#if CONFIG_MQTT_SSL == 1
    app_mbedtls_disconnect(n->SSL_Handle);
    n->SSL_Handle=NULL;
#else
    close(n->my_socket);
#endif // CONFIG_BUILD_APP_MBEDTLS
}


void NetworkInit(Network* n)
{
#if CONFIG_MQTT_SSL == 1
    n->SSL_Handle=NULL;
    n->cacert=NULL;
    n->cacertlen=0;
#else
    n->my_socket = -1;
#endif // CONFIG_BUILD_APP_MBEDTLS
    n->mqttread = Air724UG_read;
    n->mqttwrite = Air724UG_write;
    n->disconnect = Air724UG_disconnect;
}


int NetworkConnect(Network* n, char* addr, int port)
{
 int retVal = -1;
#if CONFIG_MQTT_SSL == 1

#else
    struct sockaddr_in sAddr;
    memset(&sAddr,0,sizeof(sAddr));
    struct hostent * ipAddress=NULL;

    if ((ipAddress = gethostbyname(addr)) == NULL)
        goto exit;
#endif // CONFIG_BUILD_APP_MBEDTLS

#if CONFIG_MQTT_SSL == 1
    retVal=app_mbedtls_connect(&n->SSL_Handle,addr,port,n->cacert,n->cacertlen,10000);
#else
    sAddr.sin_family=AF_INET;
    sAddr.sin_port = htons(port);
    inet_aton(ipaddr_ntoa((const openat_ip_addr_t *)ipAddress->h_addr_list[0]),&sAddr.sin_addr);

    if ((n->my_socket = socket(AF_INET,OPENAT_SOCK_STREAM, 0)) < 0)
        goto exit;

    if ((retVal = connect(n->my_socket, (const struct sockaddr *)&sAddr, sizeof(const struct sockaddr))) < 0)
    {
        close(n->my_socket);
        goto exit;
    }
#endif // CONFIG_BUILD_APP_MBEDTLS

exit:
    return retVal;
}


