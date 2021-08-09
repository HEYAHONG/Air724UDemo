#include "appsocket.hpp"
#include "appstack.hpp"
#include "debug.h"
#include "stdlib.h"
#include "stdio.h"

static __unused const char *TAG="appsocket";



class appsocket
{
private:
    appsocket_cfg_t cfg;
    int appsocket_id;
    int socketfd;
    struct
    {
        HANDLE task_handle;
    } task_info;
    static int next_appsocket_id;
public:
    //构造函数
    appsocket(appsocket_cfg_t _cfg):cfg(_cfg)
    {
        if(cfg.socket_type!=OPENAT_SOCK_STREAM && cfg.socket_type != OPENAT_SOCK_DGRAM)
        {
            cfg.socket_type=OPENAT_SOCK_STREAM;
        }
        if(cfg.task_stack_size<2048)
        {
            cfg.task_stack_size=2048;
        }
        if(cfg.task_priority==0)
        {
            cfg.task_priority=app_get_auto_task_priority();
        }

        if(next_appsocket_id<0)
        {
            next_appsocket_id=0;
        }

        appsocket_id=(next_appsocket_id++);

        socketfd=-1;

        task_info.task_handle=NULL;
    };

    //复制构造函数
    appsocket(const appsocket &other)
    {
        if(this!=&other)
        {
            cfg=other.cfg;
            appsocket_id=other.appsocket_id;
        }

        socketfd=-1;
        task_info.task_handle=NULL;
    }

    int get_appsocket_id()
    {
        return appsocket_id;
    }

    static void  task_entry(void *ptr)
    {
        appsocket &sock=(*(appsocket*)ptr);
        while(true)
        {
            sock.socketfd=socket(sock.cfg.server_addr.sin_family,sock.cfg.socket_type,0);
            if (sock.socketfd < 0)
            {
                app_debug_print("%s: appsocket %d create socket failed!\n\r",TAG,sock.appsocket_id);
                iot_os_sleep(1000);//一秒后重试
                continue;
            }
            else
            {
                if(sock.cfg.before_connect!=NULL)
                {
                    sock.cfg.before_connect(&sock.cfg,sock.socketfd);
                }
                while(true)
                {
                    int err=connect(sock.socketfd, (const struct sockaddr *)&sock.cfg.server_addr, sizeof(struct openat_sockaddr));
                    if(err<0)
                    {
                        app_debug_print("%s:appsocket %d connect error!\n\r",TAG,sock.appsocket_id);
                        close(sock.socketfd);
                        sock.socketfd=-1;
                        iot_os_sleep(3000);
                        continue;
                    }
                    else
                    {
                        if(sock.cfg.after_connect!=NULL)
                        {
                            sock.cfg.after_connect(&sock.cfg,sock.socketfd);
                        }
                        while(true)
                        {
                            if(sock.cfg.onloop!=NULL)//onloop一般不能为NULL
                            {
                                if(!sock.cfg.onloop(&sock.cfg,sock.socketfd))
                                {
                                    break;
                                }
                            }
                            iot_os_sleep(1);
                        }

                        if(sock.cfg.before_close!=NULL)
                        {
                            sock.cfg.before_close(&sock.cfg,sock.socketfd);
                        }

                        close(sock.socketfd);
                        sock.socketfd=-1;
                        break;
                    }
                }
            }

        }
    }

    //网络上线(可能会多次调用)
    void network_online()
    {
        if(task_info.task_handle==NULL)
        {
            task_info.task_handle=iot_os_create_task(task_entry,this,cfg.task_stack_size,cfg.task_priority,OPENAT_OS_CREATE_DEFAULT,(PCHAR)"appsocket");
        }
    }

    //网络离线
    void network_offline()
    {
        if(task_info.task_handle!=NULL)
        {
            iot_os_delete_task(task_info.task_handle);
        }
        if(socketfd>=0)
        {
            if(cfg.before_close!=NULL)
            {
                cfg.before_close(&cfg,socketfd);
            }
            close(socketfd);
        }
    }

    //析构函数
    ~appsocket()
    {
        network_offline();
    }

};

int appsocket::next_appsocket_id=0;

#include "vector"

//由于使用的自定义入口,C++全局变量的构造及析构需小心使用
std::vector<appsocket> *Queue_Handle=NULL;
HANDLE Queuelock=NULL;


extern "C"
{
    static __unused void appsocket_net_init()
    {
        app_debug_print("%s:appsocket init !\n\r",TAG);
    }
    static __unused void appsocket_net_loop(NetWork_State_t current_state,bool is_state_change,int8_t csq)
    {
        std::vector<appsocket> &Queue=(*Queue_Handle);
        if(is_state_change)
        {
            if(current_state!=NETWORK_STATE_CONNECTED)
            {
                app_debug_print("%s:deactive all appsocket!!!\n\r",TAG);

                iot_os_wait_semaphore(Queuelock,0);

                //遍历appsocket queue

                for(auto it=Queue.begin(); it!=Queue.end(); it++)
                {
                    (*it).network_offline();
                }

                iot_os_release_semaphore(Queuelock);
            }
            else
            {
                app_debug_print("%s:active all appsocket!!!\n\r",TAG);
            }
        }

        if(current_state==NETWORK_STATE_CONNECTED)
        {



            iot_os_wait_semaphore(Queuelock,0);

            //遍历appsocket queue

            for(auto it=Queue.begin(); it!=Queue.end(); it++)
            {
                (*it).network_online();
            }

            iot_os_release_semaphore(Queuelock);
        }
    }
}

//初始化appsocket,需要比network_init先调用
void appsocket_init()
{
    //创建锁
    if(Queuelock==NULL)
    {
        Queuelock=iot_os_create_semaphore(1);
    }

    //创建Queue(一般不会释放)
    if(Queue_Handle==NULL)
    {
        Queue_Handle=new std::vector<appsocket>;
    }

    network_callback_t cb= {appsocket_net_init,appsocket_net_loop};
    network_set_callback(cb);
}

//添加appsocket,失败返回负数
int appsocket_add(appsocket_cfg_t cfg)
{
    if(cfg.onloop==NULL || cfg.server_addr.sin_addr.s_addr==0)
    {
        return -1;
    }
    std::vector<appsocket> &Queue=(*Queue_Handle);
    appsocket sock(cfg);
    {
        //添加
        iot_os_wait_semaphore(Queuelock,0);

        Queue.push_back(sock);

        iot_os_release_semaphore(Queuelock);
    }

    return sock.get_appsocket_id();
}

bool appsocket_remove(int appsocket_id)
{
    std::vector<appsocket> &Queue=(*Queue_Handle);

    {
        //删除
        iot_os_wait_semaphore(Queuelock,0);

        for(auto it=Queue.begin(); it!=Queue.end(); it++)
        {
            if((*it).get_appsocket_id()==appsocket_id)
            {
                Queue.erase(it);
                break;
            }
        }

        iot_os_release_semaphore(Queuelock);
    }

    return false;
}
