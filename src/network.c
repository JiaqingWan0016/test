/**
 * @file network.c
 * @brief 网络接口监控实现
 */

#include "../include/network.h"
#include "../include/log.h"
#include "../include/config.h"
#include <sys/ioctl.h>

/* 检查必要的头文件 */
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif

/* 全局变量 */
static struct interface_info g_interfaces[MAX_INTERFACES];
static int g_interface_count = 0;
static int g_netlink_fd = -1; /* netlink套接字描述符 */

/* IFNAMSIZ前向声明 */
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

/**
 * @brief 创建netlink套接字
 * 
 * @return 成功返回套接字描述符，失败返回-1
 */
static int create_netlink_socket(void)
{
    int fd;
    struct sockaddr_nl addr;
    
    /* 创建netlink套接字 */
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (fd < 0) {
        LOG_ERROR("Failed to create netlink socket: %s", strerror(errno));
        return -1;
    }
    
    /* 设置非阻塞模式 */
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    
    /* 清空地址结构 */
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR;
    
    /* 绑定套接字 */
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        LOG_ERROR("Failed to bind netlink socket: %s", strerror(errno));
        close(fd);
        return -1;
    }
    
    LOG_INFO("Netlink socket created successfully, fd: %d", fd);
    return fd;
}

/**
 * @brief 获取接口IP地址
 * 
 * @param name 接口名称
 * @param info 接口信息结构指针
 * @return 成功返回SUCCESS，失败返回ERROR
 */
static int get_interface_address(const char *name, struct interface_info *info)
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    
    if (getifaddrs(&ifaddr) == -1) {
        LOG_ERROR("Failed to get interface addresses: %s", strerror(errno));
        return ERROR;
    }
    
    /* 清空IPv4和IPv6地址 */
    info->ipv4[0] = '\0';
    info->ipv6[0] = '\0';
    
    /* 遍历所有接口 */
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;
            
        /* 检查接口名称是否匹配 */
        if (strcmp(ifa->ifa_name, name) != 0)
            continue;
            
        family = ifa->ifa_addr->sa_family;
        
        /* 获取IPv4地址 */
        if (family == AF_INET) {
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &addr->sin_addr, info->ipv4, INET_ADDRSTRLEN);
        }
        /* 获取IPv6地址 */
        else if (family == AF_INET6) {
            struct sockaddr_in6 *addr = (struct sockaddr_in6 *)ifa->ifa_addr;
            inet_ntop(AF_INET6, &addr->sin6_addr, info->ipv6, INET6_ADDRSTRLEN);
        }
    }
    
    freeifaddrs(ifaddr);
    return SUCCESS;
}

/**
 * @brief 初始化接口信息
 * 
 * @param name 接口名称
 * @return 成功返回SUCCESS，失败返回ERROR
 */
static int init_interface(const char *name)
{
    int sock;
    struct ifreq ifr;
    int i = g_interface_count;
    
    if (i >= MAX_INTERFACES) {
        LOG_ERROR("Too many interfaces, max allowed: %d", MAX_INTERFACES);
        return ERROR;
    }
    
    /* 创建套接字 */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        LOG_ERROR("Failed to create socket: %s", strerror(errno));
        return ERROR;
    }
    
    /* 初始化接口请求结构 */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);
    
    /* 获取接口状态 */
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
        LOG_ERROR("Failed to get interface flags for %s: %s", name, strerror(errno));
        close(sock);
        return ERROR;
    }
    
    /* 填充接口信息结构 */
    strncpy(g_interfaces[i].name, name, IFNAMSIZ - 1);
    g_interfaces[i].status = (ifr.ifr_flags & IFF_UP) ? 1 : 0;
    
    /* 获取IP地址 */
    get_interface_address(name, &g_interfaces[i]);
    
    close(sock);
    
    LOG_INFO("Interface %s initialized: status=%d, IPv4=%s, IPv6=%s",
           name, g_interfaces[i].status, 
           g_interfaces[i].ipv4[0] ? g_interfaces[i].ipv4 : "none",
           g_interfaces[i].ipv6[0] ? g_interfaces[i].ipv6 : "none");
           
    g_interface_count++;
    return SUCCESS;
}

/**
 * @brief 查找接口索引
 * 
 * @param name 接口名称
 * @return 成功返回接口索引，失败返回-1
 */
static int find_interface(const char *name)
{
    for (int i = 0; i < g_interface_count; i++) {
        if (strcmp(g_interfaces[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief 更新接口状态和地址
 * 
 * @param name 接口名称
 * @return 成功返回SUCCESS，失败返回ERROR
 */
static int update_interface(const char *name)
{
    int idx = find_interface(name);
    if (idx < 0) {
        LOG_ERROR("Interface %s not found", name);
        return ERROR;
    }
    
    struct interface_info old_info;
    
    /* 保存旧的信息用于比较 */
    memcpy(&old_info, &g_interfaces[idx], sizeof(struct interface_info));
    
    /* 更新接口状态和地址 */
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        LOG_ERROR("Failed to create socket: %s", strerror(errno));
        return ERROR;
    }
    
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);
    
    /* 获取接口状态 */
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
        LOG_ERROR("Failed to get interface flags for %s: %s", name, strerror(errno));
        close(sock);
        return ERROR;
    }
    
    g_interfaces[idx].status = (ifr.ifr_flags & IFF_UP) ? 1 : 0;
    
    close(sock);
    
    /* 获取IP地址 */
    get_interface_address(name, &g_interfaces[idx]);
    
    /* 检查状态是否变化 */
    if (old_info.status != g_interfaces[idx].status) {
        LOG_WARN("Interface %s status changed: %s -> %s", name,
               old_info.status ? "UP" : "DOWN",
               g_interfaces[idx].status ? "UP" : "DOWN");
    }
    
    /* 检查IPv4地址是否变化 */
    if (strcmp(old_info.ipv4, g_interfaces[idx].ipv4) != 0) {
        LOG_WARN("Interface %s IPv4 address changed: %s -> %s", name,
               old_info.ipv4[0] ? old_info.ipv4 : "none",
               g_interfaces[idx].ipv4[0] ? g_interfaces[idx].ipv4 : "none");
    }
    
    /* 检查IPv6地址是否变化 */
    if (strcmp(old_info.ipv6, g_interfaces[idx].ipv6) != 0) {
        LOG_WARN("Interface %s IPv6 address changed: %s -> %s", name,
               old_info.ipv6[0] ? old_info.ipv6 : "none",
               g_interfaces[idx].ipv6[0] ? g_interfaces[idx].ipv6 : "none");
    }
    
    return SUCCESS;
}

/**
 * @brief 初始化网络监控模块
 * 
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int network_init(void)
{
    LOG_INFO("Initializing network monitoring module");
    
    /* 创建netlink套接字 */
    g_netlink_fd = create_netlink_socket();
    if (g_netlink_fd < 0) {
        LOG_ERROR("Failed to create netlink socket");
        return ERROR;
    }
    
    /* 清空接口信息数组 */
    memset(g_interfaces, 0, sizeof(g_interfaces));
    g_interface_count = 0;
    
    /* 更新接口列表 */
    if (network_update_interfaces() != SUCCESS) {
        LOG_ERROR("Failed to update interfaces");
        return ERROR;
    }
    
    return SUCCESS;
}

/**
 * @brief 更新接口列表
 * 
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int network_update_interfaces(void)
{
    struct interface_config *cfg = config_get_interfaces();
    if (!cfg) {
        LOG_ERROR("Failed to get interface configuration");
        return ERROR;
    }
    
    /* 遍历配置的接口 */
    for (int i = 0; i < cfg->interface_count; i++) {
        const char *name = cfg->interfaces[i];
        
        /* 如果接口已存在，则更新状态 */
        if (find_interface(name) >= 0) {
            update_interface(name);
        } else {
            /* 否则初始化接口 */
            init_interface(name);
        }
    }
    
    return SUCCESS;
}

/**
 * @brief 获取接口信息
 * 
 * @param name 接口名称
 * @return 接口信息结构指针，若不存在则返回NULL
 */
struct interface_info *network_get_interface(const char *name)
{
    int idx = find_interface(name);
    if (idx < 0)
        return NULL;
        
    return &g_interfaces[idx];
}

/**
 * @brief 处理网络事件
 * 
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int network_handle_event(void)
{
    char buffer[4096];
    struct nlmsghdr *nlh;
    int len;
    
    /* 读取netlink消息 */
    len = recv(g_netlink_fd, buffer, sizeof(buffer), 0);
    if (len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            /* 非阻塞模式下没有可用数据 */
            return SUCCESS;
        }
        LOG_ERROR("Failed to receive netlink message: %s", strerror(errno));
        return ERROR;
    }
    
    /* 处理netlink消息 */
    for (nlh = (struct nlmsghdr *)buffer; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
        /* 检查消息类型 */
        if (nlh->nlmsg_type == NLMSG_DONE || nlh->nlmsg_type == NLMSG_ERROR) {
            continue;
        }
        
        /* 处理地址变更消息 */
        if (nlh->nlmsg_type == RTM_NEWADDR || nlh->nlmsg_type == RTM_DELADDR ||
            nlh->nlmsg_type == RTM_NEWLINK || nlh->nlmsg_type == RTM_DELLINK) {
            
            /* 更新所有接口状态 */
            LOG_INFO("Network change detected, updating interfaces");
            network_update_interfaces();
            break;
        }
    }
    
    return SUCCESS;
}

/**
 * @brief 检查是否有活动的网络事件
 * 
 * @param fds 文件描述符集合
 * @return 有事件返回TRUE，无事件返回FALSE
 */
int network_check_events(fd_set *fds)
{
    if (g_netlink_fd < 0)
        return FALSE;
        
    return FD_ISSET(g_netlink_fd, fds);
}

/**
 * @brief 获取网络套接字文件描述符
 * 
 * @return 文件描述符
 */
int network_get_fd(void)
{
    return g_netlink_fd;
}

/**
 * @brief 打印所有监控接口的状态
 */
void network_print_status(void)
{
    LOG_INFO("------ Interface Status ------");
    for (int i = 0; i < g_interface_count; i++) {
        LOG_INFO("Interface: %s, Status: %s, IPv4: %s, IPv6: %s",
               g_interfaces[i].name,
               g_interfaces[i].status ? "UP" : "DOWN",
               g_interfaces[i].ipv4[0] ? g_interfaces[i].ipv4 : "none",
               g_interfaces[i].ipv6[0] ? g_interfaces[i].ipv6 : "none");
    }
    LOG_INFO("-----------------------------");
}

/**
 * @brief 清理网络监控模块资源
 */
void network_cleanup(void)
{
    if (g_netlink_fd >= 0) {
        close(g_netlink_fd);
        g_netlink_fd = -1;
    }
    
    LOG_INFO("Network monitoring module cleaned up");
} 