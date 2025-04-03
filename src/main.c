/**
 * @file main.c
 * @brief LINKD主程序入口
 */

/* 包含自动生成的配置头文件 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* 检查系统头文件 */
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "log.h"
#include "common.h"
#include "config.h"
#include "network.h"
#include "timer.h"
#include "socket.h"
#include "linkd.h"

/* 全局变量 */
static struct {
    IFBIND_CONF_HEAD conf_head;
    IFBINDCONF_NAME *conf_items;
    int netlink_fd;
    int timer_interval;
    int daemon_mode;
    FILE *log_fp;
    int log_level;
    struct sharememory *shm;
} g_ctx;

/* 守护进程化 */
int daemonize(void)
{
    pid_t pid;
    
    /* 第一次fork，创建子进程 */
    pid = fork();
    if (pid < 0) {
        log_write(LOG_LEVEL_ERROR, "fork failed: %s", strerror(errno));
        return -1;
    } else if (pid > 0) {
        exit(0);  /* 父进程退出 */
    }
    
    /* 创建新会话 */
    if (setsid() < 0) {
        log_write(LOG_LEVEL_ERROR, "setsid failed: %s", strerror(errno));
        return -1;
    }
    
    /* 第二次fork，确保进程不会获得控制终端 */
    pid = fork();
    if (pid < 0) {
        log_write(LOG_LEVEL_ERROR, "fork failed: %s", strerror(errno));
        return -1;
    } else if (pid > 0) {
        exit(0);  /* 父进程退出 */
    }
    
    /* 设置工作目录 */
    chdir("/");
    
    /* 设置文件权限掩码 */
    umask(0);
    
    /* 关闭所有打开的文件描述符 */
    for (int i = 0; i < sysconf(_SC_OPEN_MAX); i++) {
        close(i);
    }
    
    /* 重定向标准输入输出到/dev/null */
    open("/dev/null", O_RDWR);
    dup(0);
    dup(0);
    
    return 0;
}

/* 清理资源 */
void cleanup_resources(void)
{
    if (g_ctx.conf_items) {
        free(g_ctx.conf_items);
    }
    if (g_ctx.netlink_fd >= 0) {
        close(g_ctx.netlink_fd);
    }
    if (g_ctx.log_fp) {
        fclose(g_ctx.log_fp);
    }
    if (g_ctx.shm) {
        deleteshm();
    }
}

/* 主程序入口 */
int main(int argc, char *argv[])
{
    int opt;
    int ret;
    
    /* 初始化日志系统 */
    if (init_log("/tmp/.linkd_runlog", LOG_LEVEL_INFO) < 0) {
        fprintf(stderr, "Failed to initialize log system\n");
        return -1;
    }
    
    /* 解析命令行参数 */
    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
            case 'd':
                g_ctx.daemon_mode = 1;
                break;
            default:
                log_write(LOG_LEVEL_ERROR, "Invalid option: %c", opt);
                return -1;
        }
    }
    
    /* 守护进程化 */
    if (g_ctx.daemon_mode) {
        if (daemonize() < 0) {
            log_write(LOG_LEVEL_ERROR, "Failed to daemonize");
            return -1;
        }
    }
    
    /* 创建PID文件 */
    FILE *pid_fp = fopen("/var/run/linkd.pid", "w");
    if (pid_fp) {
        fprintf(pid_fp, "%d\n", getpid());
        fclose(pid_fp);
    }
    
    /* 初始化共享内存 */
    if (init_shared_memory() < 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to initialize shared memory");
        return -1;
    }
    
    /* 加载配置文件 */
    if (load_config(IFBIND_CONF_PATH, &g_ctx.conf_head, &g_ctx.conf_items) < 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to load configuration");
        return -1;
    }
    
    /* 初始化netlink */
    if (init_netlink() < 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to initialize netlink");
        return -1;
    }
    
    /* 初始化定时器 */
    g_ctx.timer_interval = 20;  /* 默认20秒 */
    if (init_timer(g_ctx.timer_interval) < 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to initialize timer");
        return -1;
    }
    
    /* 主循环 */
    while (1) {
        /* 处理netlink事件 */
        struct nl_msg *msg;
        struct nlmsghdr *nlh;
        char buf[4096];
        
        int len = recv(g_ctx.netlink_fd, buf, sizeof(buf), 0);
        if (len < 0) {
            if (errno == EINTR) {
                continue;
            }
            log_write(LOG_LEVEL_ERROR, "Failed to receive netlink message: %s", strerror(errno));
            continue;
        }
        
        nlh = (struct nlmsghdr *)buf;
        while (NLMSG_OK(nlh, len)) {
            msg = nlmsg_alloc();
            if (!msg) {
                log_write(LOG_LEVEL_ERROR, "Failed to allocate netlink message");
                break;
            }
            
            if (nlmsg_append(msg, nlh, 0, NLMSG_ALIGNTO) < 0) {
                log_write(LOG_LEVEL_ERROR, "Failed to append netlink message");
                nlmsg_free(msg);
                break;
            }
            
            handle_netlink_event(msg, NULL);
            nlmsg_free(msg);
            
            nlh = NLMSG_NEXT(nlh, len);
        }
    }
    
    /* 清理资源 */
    cleanup_resources();
    
    return 0;
} 