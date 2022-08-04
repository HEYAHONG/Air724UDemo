
#if !defined(__MQTTAir724UG__H)
#define __MQTTAir724UG__H

#include "iot_os.h"
#include "stdint.h"
#include "kconfig.h"

typedef struct Timer
{
    uint64_t start_tick;

    int timeout_ms;

} Timer;

typedef struct Network Network;

struct Network
{
#if  CONFIG_BUILD_APP_MBEDTLS == 1
    void * SSL_Handle;
    const char *cacert;
    size_t cacertlen;
#else
	int my_socket;
#endif
	int (*mqttread) (Network*, unsigned char*, int, int);
	int (*mqttwrite) (Network*, unsigned char*, int, int);
	void (*disconnect) (Network*);
};

void TimerInit(Timer*);
char TimerIsExpired(Timer*);
void TimerCountdownMS(Timer*, unsigned int);
void TimerCountdown(Timer*, unsigned int);
int TimerLeftMS(Timer*);

typedef struct Mutex
{
    HANDLE sem;
} Mutex;

void MutexInit(Mutex*);
int MutexLock(Mutex*);
int MutexUnlock(Mutex*);

typedef struct Thread
{

} Thread;

int ThreadStart(Thread*, void (*fn)(void*), void* arg);

int Air724UG_read(Network*, unsigned char*, int, int);
int Air724UG_write(Network*, unsigned char*, int, int);
void Air724UG_disconnect(Network*);

void NetworkInit(Network*);
int NetworkConnect(Network*, char*, int);

#endif
