/**
 * @file network.h
 * @brief 网络接口监控
 */
#ifndef _NETWORK_H
#define _NETWORK_H

#include "common.h"

/* 检查必要的头文件 */
#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_LINUX_NETLINK_H
#include <linux/netlink.h>
#endif
#ifdef HAVE_LINUX_RTNETLINK_H
#include <linux/rtnetlink.h>
#endif

/**
 * @brief 接口信息结构
 */
struct interface_info {
    char name[IFNAMSIZ];        /* 接口名称 */
    char ipv4[INET_ADDRSTRLEN];    /* IPv4地址 */
    char ipv6[INET6_ADDRSTRLEN];   /* IPv6地址 */
    int status;                    /* 接口状态，1表示启用，0表示禁用 */
};

/**
 * @brief 初始化网络监控模块
 * 
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int network_init(void);

/**
 * @brief 更新接口列表
 * 
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int network_update_interfaces(void);

/**
 * @brief 获取接口信息
 * 
 * @param name 接口名称
 * @return 接口信息结构指针，若不存在则返回NULL
 */
struct interface_info *network_get_interface(const char *name);

/**
 * @brief 处理网络事件
 * 
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int network_handle_event(void);

/**
 * @brief 检查是否有活动的网络事件
 * 
 * @param fds 文件描述符集合
 * @return 有事件返回TRUE，无事件返回FALSE
 */
int network_check_events(fd_set *fds);

/**
 * @brief 获取网络套接字文件描述符
 * 
 * @return 文件描述符
 */
int network_get_fd(void);

/**
 * @brief 打印所有监控接口的状态
 */
void network_print_status(void);

/**
 * @brief 清理网络监控模块资源
 */
void network_cleanup(void);

#endif /* _NETWORK_H */ 