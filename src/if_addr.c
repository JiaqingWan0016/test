#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_addr.h>
#include <arpa/inet.h>
#include "linkd.h"
#include "if_addr.h"

/* 获取接口IPv4地址 */
int get_if_ipv4_addr(const char *if_name, struct if_ipv4_addr *ipv4, uint32_t specified_addr)
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    int sock;
    int ret = -1;
    
    /* 创建socket */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to create socket: %s", strerror(errno));
        return -1;
    }
    
    /* 获取接口配置 */
    memset(&ifc, 0, sizeof(ifc));
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    
    if (ioctl(sock, SIOCGIFCONF, &ifc) < 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to get interface config: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    /* 遍历所有接口地址 */
    struct ifreq *ifr_ptr = ifc.ifc_req;
    int n = ifc.ifc_len / sizeof(struct ifreq);
    
    for (int i = 0; i < n; i++) {
        if (strcmp(ifr_ptr[i].ifr_name, if_name) == 0) {
            struct sockaddr_in *addr = (struct sockaddr_in *)&ifr_ptr[i].ifr_addr;
            struct sockaddr_in *netmask = (struct sockaddr_in *)&ifr_ptr[i].ifr_netmask;
            
            /* 如果指定了地址，检查是否匹配 */
            if (specified_addr != 0 && addr->sin_addr.s_addr != specified_addr) {
                continue;
            }
            
            /* 找到匹配的地址，保存信息 */
            ipv4->addr = addr->sin_addr.s_addr;
            ipv4->netmask = netmask->sin_addr.s_addr;
            ret = 0;
            break;
        }
    }
    
    /* 如果没有找到匹配的地址，设置为0 */
    if (ret < 0) {
        ipv4->addr = 0;
        ipv4->netmask = 0;
    }
    
    close(sock);
    return ret;
}

/* 获取接口IPv6地址 */
int get_if_ipv6_addr(const char *if_name, struct if_ipv6_addr *ipv6, const uint32_t *specified_addr)
{
    struct nlmsghdr *nlh;
    struct ifaddrmsg *ifa;
    struct rtattr *rta;
    char buf[8192];
    int sock;
    int ret = -1;
    
    /* 创建netlink socket */
    sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sock < 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to create netlink socket: %s", strerror(errno));
        return -1;
    }
    
    /* 准备请求消息 */
    memset(buf, 0, sizeof(buf));
    nlh = (struct nlmsghdr *)buf;
    nlh->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
    nlh->nlmsg_type = RTM_GETADDR;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nlh->nlmsg_seq = 1;
    nlh->nlmsg_pid = 0;
    
    ifa = (struct ifaddrmsg *)(nlh + 1);
    ifa->ifa_family = AF_INET6;
    ifa->ifa_prefixlen = 0;
    ifa->ifa_flags = 0;
    ifa->ifa_scope = 0;
    ifa->ifa_index = 0;
    
    /* 发送请求 */
    if (send(sock, nlh, nlh->nlmsg_len, 0) < 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to send netlink request: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    /* 接收响应 */
    int len;
    while ((len = recv(sock, buf, sizeof(buf), 0)) > 0) {
        nlh = (struct nlmsghdr *)buf;
        while (NLMSG_OK(nlh, len)) {
            if (nlh->nlmsg_type == NLMSG_DONE) {
                break;
            }
            
            ifa = (struct ifaddrmsg *)NLMSG_DATA(nlh);
            
            /* 获取接口名称 */
            char if_name_buf[IFNAMSIZ];
            if_indextoname(ifa->ifa_index, if_name_buf);
            
            /* 检查是否是目标接口 */
            if (strcmp(if_name_buf, if_name) == 0) {
                /* 遍历地址属性 */
                rta = IFA_RTA(ifa);
                int rta_len = IFA_PAYLOAD(nlh);
                
                while (RTA_OK(rta, rta_len)) {
                    if (rta->rta_type == IFA_ADDRESS) {
                        struct in6_addr *addr = (struct in6_addr *)RTA_DATA(rta);
                        
                        /* 检查是否是链路本地地址（以fe80::开头） */
                        if (addr->s6_addr[0] == 0xfe && addr->s6_addr[1] == 0x80) {
                            log_write(LOG_LEVEL_DEBUG, "Found link-local IPv6 address, skipping");
                            rta = RTA_NEXT(rta, rta_len);
                            continue;
                        }
                        
                        /* 如果指定了地址，检查是否匹配 */
                        if (specified_addr != NULL) {
                            if (memcmp(addr->s6_addr32, specified_addr, sizeof(addr->s6_addr32)) != 0) {
                                rta = RTA_NEXT(rta, rta_len);
                                continue;
                            }
                        }
                        
                        /* 找到匹配的地址，保存信息 */
                        memcpy(ipv6->addr, addr->s6_addr32, sizeof(ipv6->addr));
                        ret = 0;
                        goto out;
                    }
                    rta = RTA_NEXT(rta, rta_len);
                }
            }
            
            nlh = NLMSG_NEXT(nlh, len);
        }
    }
    
out:
    /* 如果没有找到匹配的地址，设置为0 */
    if (ret < 0) {
        memset(ipv6->addr, 0, sizeof(ipv6->addr));
    }
    
    close(sock);
    return ret;
} 