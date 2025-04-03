#ifndef IF_ADDR_H
#define IF_ADDR_H

#include <stdint.h>
#include <netinet/in.h>

/* IPv4地址信息结构 */
struct if_ipv4_addr {
    uint32_t addr;      /* 地址 */
    uint32_t netmask;   /* 掩码 */
};

/* IPv6地址信息结构 */
struct if_ipv6_addr {
    uint32_t addr[4];   /* IPv6地址 */
};

/* 获取接口IPv4地址
 * @param if_name: 接口名称
 * @param addr: 指定的IPv4地址（如果为0，则使用第一个地址）
 * @return: 成功返回0，失败返回-1
 */
int get_if_ipv4_addr(const char *if_name, struct if_ipv4_addr *ipv4, uint32_t specified_addr);

/* 获取接口IPv6地址
 * @param if_name: 接口名称
 * @param addr: 指定的IPv6地址（如果为NULL，则使用第一个地址）
 * @return: 成功返回0，失败返回-1
 */
int get_if_ipv6_addr(const char *if_name, struct if_ipv6_addr *ipv6, const uint32_t *specified_addr);

#endif /* IF_ADDR_H */ 