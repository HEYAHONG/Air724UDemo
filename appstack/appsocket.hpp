#ifndef APPSOCKET_HPP_INCLUDED
#define APPSOCKET_HPP_INCLUDED



#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "network.h"
#include "iot_socket.h"
#include "iot_os.h"
#include "stdint.h"

/*
appsocket主要负责在联网时打开socket,在失去连接时断开socket，收发操作仍然通过socket操作进行
*/

//初始化appsocket,需要比network_init先调用
void appsocket_init();

/*
appsocket_cfg 客户端配置,通常2G/3G/4G 一般不会使用服务器模式。
*/
typedef struct __appsocket_cfg_t
{
    //服务器地址
    struct openat_sockaddr_in server_addr;
    //套接字类型
    int socket_type;//只能是OPENAT_SOCK_STREAM/OPENAT_SOCK_DGRAM,不为有效值时按OPENAT_SOCK_STREAM
    //套接字任务栈大小
    uint32_t task_stack_size;//小于2048按2048
    //套接字任务优先级
    uint32_t task_priority;//为0时自动分配
    //连接前回调函数
    void (*before_connect)(const struct __appsocket_cfg_t * cfg,int socket_fd);
    //连接成功后回调函数
    void (*after_connect)(const struct __appsocket_cfg_t *cfg,int socketfd);
    //成功连接后循环内回调函数(只能进行发送与接收操作),不可长时间阻塞
    bool (*onloop)(const struct __appsocket_cfg_t *cfg,int socketfd);//返回false重启socket
    //关闭前回调
    void (*before_close)(const struct __appsocket_cfg_t *cfg,int socketfd);

} appsocket_cfg_t;


//通过点分十进制IP地址获得地址struct openat_sockaddr_in
struct openat_sockaddr_in appsocket_get_addr_by_ip(const char * ip,uint16_t port);


//添加appsocket,失败返回负数,成功后返回appsocket_id(可用于删除)
int appsocket_add(appsocket_cfg_t cfg);


bool appsocket_remove(int appsocket_id);


#ifdef __cplusplus
};
#endif // __cplusplus

#endif // APPSOCKET_HPP_INCLUDED
