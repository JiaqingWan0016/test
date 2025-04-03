#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "linkd.h"
#include "if_sync.h"
#include "if_addr.h"

/* 执行ipsec接口的down/up操作 */
static int ipsec_if_down_up(const char *if_name)
{
    char cmd[256];
    int ret;
    
    /* 执行down操作 */
    snprintf(cmd, sizeof(cmd), "ifconfig %s down", if_name);
    ret = system(cmd);
    if (ret != 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to bring down interface %s: %s", if_name, strerror(errno));
        return -1;
    }
    
    /* 等待一小段时间 */
    usleep(100000);  /* 100ms */
    
    /* 执行up操作 */
    snprintf(cmd, sizeof(cmd), "ifconfig %s up", if_name);
    ret = system(cmd);
    if (ret != 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to bring up interface %s: %s", if_name, strerror(errno));
        return -1;
    }
    
    /* 执行whack命令 */
    ret = system("/tos/bin/ipsec-cmd/whack --listen > /dev/null 2>/dev/null");
    if (ret != 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to execute whack command: %s", strerror(errno));
        return -1;
    }
    
    return 0;
}

/* 同步接口状态 */
int sync_interface_state(const char *if_name)
{
    struct ifreq ifr;
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
            
            /* 获取IPv4地址 */
            struct if_ipv4_addr ipv4;
            if (get_if_ipv4_addr(if_name, &ipv4, item->ibc.ip) >= 0) {
                new_info.interfaceip = ipv4.addr;
                new_info.netmask = ipv4.netmask;
            }
            
            /* 获取IPv6地址 */
            struct if_ipv6_addr ipv6;
            if (get_if_ipv6_addr(if_name, &ipv6, item->ibc.ipv6) >= 0) {
                memcpy(new_info.ipv6, ipv6.addr, sizeof(new_info.ipv6));
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
            if (changes) {
                log_write(LOG_LEVEL_INFO, "Interface %s information changed, updating shared memory", if_name);
                
                /* 更新共享内存 */
                if (update_shared_memory(&new_info) < 0) {
                    log_write(LOG_LEVEL_ERROR, "Failed to update shared memory");
                }
                
                /* 通知vdcd进程 */
                if (notify_vdcd_process() < 0) {
                    log_write(LOG_LEVEL_ERROR, "Failed to notify vdcd process");
                }
                
                /* 同步到ipsec接口 */
                char cmd[512];
                
                /* 设置ipsec接口的IPv4地址和掩码 */
                if (new_info.interfaceip != 0) {
                    struct in_addr addr;
                    addr.s_addr = new_info.interfaceip;
                    struct in_addr mask;
                    mask.s_addr = new_info.netmask;
                    
                    snprintf(cmd, sizeof(cmd), "ifconfig %s %s netmask %s", 
                            item->if_name,
                            inet_ntoa(addr),
                            inet_ntoa(mask));
                    if (system(cmd) != 0) {
                        log_write(LOG_LEVEL_ERROR, "Failed to set IPv4 address for %s", item->if_name);
                    }
                }
                
                /* 设置ipsec接口的IPv6地址 */
                if (memcmp(new_info.ipv6, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16) != 0) {
                    char ipv6_str[INET6_ADDRSTRLEN];
                    struct in6_addr addr;
                    memcpy(addr.s6_addr32, new_info.ipv6, sizeof(addr.s6_addr32));
                    
                    inet_ntop(AF_INET6, &addr, ipv6_str, sizeof(ipv6_str));
                    snprintf(cmd, sizeof(cmd), "ifconfig %s inet6 add %s/128", 
                            item->if_name, ipv6_str);
                    if (system(cmd) != 0) {
                        log_write(LOG_LEVEL_ERROR, "Failed to set IPv6 address for %s", item->if_name);
                    }
                }
                
                /* 设置ipsec接口的MTU */
                snprintf(cmd, sizeof(cmd), "ifconfig %s mtu %d", 
                        item->if_name, new_info.mtu);
                if (system(cmd) != 0) {
                    log_write(LOG_LEVEL_ERROR, "Failed to set MTU for %s", item->if_name);
                }
                
                /* 执行ipsec接口的down/up操作和whack命令 */
                if (ipsec_if_down_up(item->if_name) < 0) {
                    log_write(LOG_LEVEL_ERROR, "Failed to bring down/up interface %s", item->if_name);
                }
            } else {
                log_write(LOG_LEVEL_DEBUG, "Interface %s information unchanged, skipping update", if_name);
            }
        }
    }
    
    close(sock);
    return 0;
} 