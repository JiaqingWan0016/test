#ifndef _LINKD_H_
#define _LINKD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <pthread.h>
#include <time.h>
#include <syslog.h>

/* 配置相关定义 */
#define IFBIND_CONF_PATH "/tos/conf/vpn/ifbind.conf"
#define MAX_IPSEC_INTERFACES 4
#define PHYSICALIF_LEN 16

/* 文件头部结构 */
typedef struct {
    char magic[4];
    unsigned long item_num;
    int id;
    char service_flag;
    unsigned char reserved[7];
} IFBIND_CONF_HEAD;

/* IP地址结构 */
typedef struct {
    union {
        struct sockaddr_in v4;
        struct sockaddr_in6 v6;
    } u;
#define ip_addr_v4          u.v4.sin_addr.s_addr
#define ip_addr_v6          u.v6.sin6_addr.s6_addr32
#define ip_family           u.v4.sin_family
} ip_address;

/* 接口绑定配置结构 */
typedef struct {
    unsigned char linkpriority;
    unsigned char reserved[3];
    int status;
    char dev[IFNAMSIZ];
    __u32 ip;
    __u32 ip_mask;
    ip_address forceip;
    __u32 ipv6[4];
    int id;
} IFBIND_CONF;

/* 接口绑定配置名称结构 */
typedef struct {
    char if_name[IFNAMSIZ];
    IFBIND_CONF ibc;
} IFBINDCONF_NAME;

/* 链路信息结构 */
typedef struct {
    unsigned char linkpriority;
    unsigned char linkstate;
    unsigned char linkstate_v6;
    unsigned char reserved[2];
    char virtualinterface[PHYSICALIF_LEN];
    char physical[PHYSICALIF_LEN];
    char realinterface[PHYSICALIF_LEN];
    unsigned long interfaceip;
    unsigned long netmask;
    unsigned long gatewayip;
    ip_address forceip;
    unsigned long gwipv6[4];
    unsigned long ipv6[4];
    unsigned long prefix;
    unsigned long mtu;
} linkinfo;

/* 共享内存结构 */
typedef struct {
    unsigned char linkscount;
    unsigned char reserved[3];
    struct linkinfo link[4];
    char pname[4][16];
    pid_t pid[4];
} sharememory;

/* 日志级别定义 */
#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_ERROR 3

/* 函数声明 */
/* 配置相关 */
int load_config(const char *conf_path, IFBIND_CONF_HEAD *head, IFBINDCONF_NAME **items);
int reload_config(void);
int validate_config(const IFBIND_CONF_HEAD *head, const IFBINDCONF_NAME *items);

/* Netlink相关 */
int init_netlink(void);
int handle_netlink_event(struct nl_msg *msg, void *arg);
int sync_interface_state(const char *if_name);

/* 定时任务相关 */
int init_timer(int interval);
int update_timer_interval(int new_interval);
void timer_task_handler(void *arg);

/* 共享内存相关 */
int init_shared_memory(void);
int update_shared_memory(const struct linkinfo *info);
int notify_vdcd_process(void);

/* 日志相关 */
int init_log(const char *log_path, int level);
void log_write(int level, const char *fmt, ...);
void log_rotate(void);

/* 错误处理相关 */
int error_recovery(int error_type);
void cleanup_resources(void);

/* 主程序相关 */
int daemonize(void);
int main(int argc, char *argv[]);

#endif /* _LINKD_H_ */ 