#include "linkd.h"
#include "if_sync.h"

/* 初始化netlink */
int init_netlink(void)
{
    struct sockaddr_nl addr;
    int ret;
    
    /* 创建netlink socket */
    g_ctx.netlink_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (g_ctx.netlink_fd < 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to create netlink socket: %s", strerror(errno));
        return -1;
    }
    
    /* 设置地址 */
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR | RTMGRP_NEIGH;
    
    /* 绑定socket */
    ret = bind(g_ctx.netlink_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to bind netlink socket: %s", strerror(errno));
        close(g_ctx.netlink_fd);
        return -1;
    }
    
    log_write(LOG_LEVEL_INFO, "Successfully initialized netlink");
    return 0;
}

/* 处理netlink事件 */
int handle_netlink_event(struct nl_msg *msg, void *arg)
{
    struct nlmsghdr *nlh = nlmsg_hdr(msg);
    struct ifaddrmsg *ifa;
    struct ifinfomsg *ifi;
    struct ndmsg *ndm;
    char if_name[IFNAMSIZ];
    
    switch (nlh->nlmsg_type) {
        case RTM_NEWLINK:
        case RTM_DELLINK:
            ifi = NLMSG_DATA(nlh);
            if_indextoname(ifi->ifi_index, if_name);
            log_write(LOG_LEVEL_INFO, "Interface %s %s", if_name,
                     nlh->nlmsg_type == RTM_NEWLINK ? "up" : "down");
            sync_interface_state(if_name);
            break;
            
        case RTM_NEWADDR:
        case RTM_DELADDR:
            ifa = NLMSG_DATA(nlh);
            if_indextoname(ifa->ifa_index, if_name);
            log_write(LOG_LEVEL_INFO, "Interface %s %s address %s", if_name,
                     ifa->ifa_family == AF_INET ? "IPv4" : "IPv6",
                     nlh->nlmsg_type == RTM_NEWADDR ? "added" : "removed");
            sync_interface_state(if_name);
            break;
            
        case RTM_DELNEIGH:
            ndm = NLMSG_DATA(nlh);
            if_indextoname(ndm->ndm_ifindex, if_name);
            log_write(LOG_LEVEL_INFO, "Interface %s neighbor deleted", if_name);
            sync_interface_state(if_name);
            break;
    }
    
    return 0;
}

/* 同步接口状态 */
int sync_interface_state(const char *if_name)
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    int sock;
    int i;
    
    /* 创建socket */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to create socket: %s", strerror(errno));
        return -1;
    }
    
    /* 获取接口信息 */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ - 1);
    
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to get interface flags: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    /* 获取接口地址 */
    memset(&ifc, 0, sizeof(ifc));
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    
    if (ioctl(sock, SIOCGIFCONF, &ifc) < 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to get interface config: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    /* 检查配置数量是否发生变化 */
    struct linkinfo old_info;
    int config_changes = 0;
    if (read_shared_memory(&old_info) >= 0) {
        if (old_info.linkpriority != g_ctx.conf_head.item_num) {
            log_write(LOG_LEVEL_INFO, "Configuration count changed: %d -> %d", 
                     old_info.linkpriority, g_ctx.conf_head.item_num);
            config_changes = 1;
        }
    } else {
        /* 如果无法读取共享内存，认为配置已变化 */
        config_changes = 1;
    }
    
    /* 遍历所有ipsec接口 */
    for (i = 0; i < g_ctx.conf_head.item_num; i++) {
        const IFBINDCONF_NAME *item = &g_ctx.conf_items[i];
        
        /* 检查是否绑定到当前接口 */
        if (strcmp(item->ibc.dev, if_name) == 0) {
            struct linkinfo new_info;
            struct linkinfo old_info;
            int changes = 0;
            
            /* 初始化新的链路信息 */
            memset(&new_info, 0, sizeof(new_info));
            new_info.linkpriority = item->ibc.linkpriority;
            strncpy(new_info.virtualinterface, item->if_name, PHYSICALIF_LEN - 1);
            strncpy(new_info.physical, item->ibc.dev, PHYSICALIF_LEN - 1);
            
            /* 获取当前接口状态 */
            new_info.linkstate = (ifr.ifr_flags & IFF_UP) ? 1 : 0;
            
            /* 获取当前MTU */
            if (ioctl(sock, SIOCGIFMTU, &ifr) >= 0) {
                new_info.mtu = ifr.ifr_mtu;
            }
            
            /* 获取当前IPv4地址 */
            struct ifreq ifr_ip;
            memset(&ifr_ip, 0, sizeof(ifr_ip));
            strncpy(ifr_ip.ifr_name, if_name, IFNAMSIZ - 1);
            
            if (ioctl(sock, SIOCGIFADDR, &ifr_ip) >= 0) {
                struct sockaddr_in *addr = (struct sockaddr_in *)&ifr_ip.ifr_addr;
                new_info.interfaceip = addr->sin_addr.s_addr;
            }
            
            /* 获取当前IPv4掩码 */
            if (ioctl(sock, SIOCGIFNETMASK, &ifr_ip) >= 0) {
                struct sockaddr_in *addr = (struct sockaddr_in *)&ifr_ip.ifr_netmask;
                new_info.netmask = addr->sin_addr.s_addr;
            }
            
            /* 获取当前IPv6地址 */
            int sock6 = socket(AF_INET6, SOCK_DGRAM, 0);
            if (sock6 >= 0) {
                struct in6_ifreq ifr6;
                memset(&ifr6, 0, sizeof(ifr6));
                strncpy(ifr6.ifr_name, if_name, IFNAMSIZ - 1);
                
                if (ioctl(sock6, SIOCGIFADDR, &ifr6) >= 0) {
                    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&ifr6.ifr_addr;
                    memcpy(new_info.ipv6, addr6->sin6_addr.s6_addr32, sizeof(new_info.ipv6));
                }
                
                close(sock6);
            }
            
            /* 读取共享内存中的旧信息 */
            if (read_shared_memory(&old_info) >= 0) {
                /* 比较信息变化 */
                if (strcmp(old_info.virtualinterface, new_info.virtualinterface) != 0) {
                    log_write(LOG_LEVEL_INFO, "Virtual interface changed: %s -> %s", 
                             old_info.virtualinterface, new_info.virtualinterface);
                    changes = 1;
                }
                if (strcmp(old_info.physical, new_info.physical) != 0) {
                    log_write(LOG_LEVEL_INFO, "Physical interface changed: %s -> %s", 
                             old_info.physical, new_info.physical);
                    changes = 1;
                }
                if (old_info.linkstate != new_info.linkstate) {
                    log_write(LOG_LEVEL_INFO, "Link state changed: %d -> %d", 
                             old_info.linkstate, new_info.linkstate);
                    changes = 1;
                }
                if (old_info.mtu != new_info.mtu) {
                    log_write(LOG_LEVEL_INFO, "MTU changed: %d -> %d", 
                             old_info.mtu, new_info.mtu);
                    changes = 1;
                }
                if (old_info.interfaceip != new_info.interfaceip) {
                    log_write(LOG_LEVEL_INFO, "IPv4 address changed: %u -> %u", 
                             old_info.interfaceip, new_info.interfaceip);
                    changes = 1;
                }
                if (old_info.netmask != new_info.netmask) {
                    log_write(LOG_LEVEL_INFO, "IPv4 netmask changed: %u -> %u", 
                             old_info.netmask, new_info.netmask);
                    changes = 1;
                }
                if (memcmp(old_info.ipv6, new_info.ipv6, sizeof(new_info.ipv6)) != 0) {
                    log_write(LOG_LEVEL_INFO, "IPv6 address changed");
                    changes = 1;
                }
            } else {
                /* 如果无法读取共享内存，认为信息已变化 */
                changes = 1;
            }
            
            /* 只有在信息发生变化时才更新共享内存和通知vdcd */
            if (changes || config_changes) {
                log_write(LOG_LEVEL_INFO, "Interface %s information changed, updating shared memory", if_name);
                
                /* 更新共享内存 */
                if (update_shared_memory(&new_info) < 0) {
                    log_write(LOG_LEVEL_ERROR, "Failed to update shared memory");
                }
                
                /* 通知vdcd进程 */
                if (notify_vdcd_process() < 0) {
                    log_write(LOG_LEVEL_ERROR, "Failed to notify vdcd process");
                }
            } else {
                log_write(LOG_LEVEL_DEBUG, "Interface %s information unchanged, skipping update", if_name);
            }
        }
    }
    
    close(sock);
    return 0;
} 