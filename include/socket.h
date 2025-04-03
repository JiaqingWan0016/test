/**
 * @file socket.h
 * @brief 本地套接字通信
 */
#ifndef _SOCKET_H
#define _SOCKET_H

#include "common.h"

/* 检查必要的头文件 */
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif

/**
 * @brief 初始化本地套接字服务
 * 
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int socket_init(void);

/**
 * @brief 处理套接字命令
 * 
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int socket_handle_command(void);

/**
 * @brief 检查是否有活动的套接字连接
 * 
 * @param fds 文件描述符集合
 * @return 有连接返回TRUE，无连接返回FALSE
 */
int socket_check_events(fd_set *fds);

/**
 * @brief 获取套接字文件描述符
 * 
 * @return 文件描述符
 */
int socket_get_fd(void);

/**
 * @brief 清理套接字资源
 */
void socket_cleanup(void);

#endif /* _SOCKET_H */ 