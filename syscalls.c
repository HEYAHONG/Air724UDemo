/* Includes */
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include "iot_debug.h"
#include "iot_os.h"




char *__env[1] = { 0 };
char **environ = __env;


/* Functions */
void initialise_monitor_handles()
{
}

int _getpid(void)
{
    return 1;
}

int _kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}

void _exit (int status)
{
    _kill(status, -1);
    while (1) {}		/* Make sure we hang here */
}



extern int read_tty(char *ptr,int len);
__attribute__((weak)) int _read(int file, char *ptr, int len)
{

    if(ptr==NULL)
    {
        return 0;
    }

    if(file==0)
    {
        return read_tty(ptr,len);
    }

    return 0;
}

extern int write_tty(char *ptr,int len);
__attribute__((weak)) int _write(int file, char *ptr, int len)
{
    if(ptr==NULL)
    {
        return 0;
    }

    if(file==1 || file == 2)
    {
        return write_tty(ptr,len);
    }

    return 0;
}

int _close(int file)
{
    return -1;
}


int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file)
{
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    return 0;
}

int _open(char *path, int flags, ...)
{
    /* Pretend like we always fail */
    return -1;
}

int _wait(int *status)
{
    errno = ECHILD;
    return -1;
}

int _unlink(char *name)
{
    errno = ENOENT;
    return -1;
}


int _stat(char *file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _link(char *old, char *new)
{
    errno = EMLINK;
    return -1;
}

int _fork(void)
{
    errno = EAGAIN;
    return -1;
}

int _execve(char *name, char **argv, char **env)
{
    errno = ENOMEM;
    return -1;
}

time_t time(time_t *timer)
{
    time_t time=IVTBL(get_timestamp)();
    if(timer)
    {
        (*timer)=time;
    }
    return time;
}
