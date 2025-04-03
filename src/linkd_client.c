/**
 * @file linkd_client.c
 * @brief LINKD客户端工具
 */

/* 包含自动生成的配置头文件 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* 检查系统头文件的存在性 */
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif

#define SOCKET_PATH "/tmp/linkd_socket"

/* 命令类型 */
#define CMD_UPDATE_INTERVAL 1
#define CMD_EXIT 2

/* 命令格式 */
struct command {
    int type;
    union {
        unsigned int interval;
    } data;
};

/**
 * @brief 打印使用帮助
 * 
 * @param prog_name 程序名称
 */
static void print_usage(const char *prog_name)
{
    printf("Usage: %s <command> [args]\n", prog_name);
    printf("Commands:\n");
    printf("  interval <seconds>   设置定时间隔（秒）\n");
    printf("  exit                 退出LINKD守护进程\n");
    printf("  help                 显示帮助信息\n");
}

/**
 * @brief 发送命令到LINKD
 * 
 * @param cmd 命令结构
 * @return 成功返回0，失败返回非0
 */
static int send_command(struct command *cmd)
{
    int sockfd;
    struct sockaddr_un addr;
    char buffer[128];
    
    /* 创建本地套接字 */
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }
    
    /* 设置地址 */
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    
    /* 连接服务器 */
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sockfd);
        return 1;
    }
    
    /* 发送命令 */
    if (send(sockfd, cmd, sizeof(*cmd), 0) != sizeof(*cmd)) {
        perror("send");
        close(sockfd);
        return 1;
    }
    
    /* 接收响应 */
    int n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        printf("服务器响应: %s\n", buffer);
    }
    
    close(sockfd);
    return 0;
}

/**
 * @brief 主函数
 * 
 * @param argc 参数数量
 * @param argv 参数数组
 * @return 成功返回0，失败返回非0
 */
int main(int argc, char *argv[])
{
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    /* 处理help命令 */
    if (strcmp(argv[1], "help") == 0) {
        print_usage(argv[0]);
        return 0;
    }
    
    struct command cmd;
    
    /* 处理interval命令 */
    if (strcmp(argv[1], "interval") == 0) {
        if (argc < 3) {
            printf("错误: 缺少间隔时间参数\n");
            return 1;
        }
        
        int interval = atoi(argv[2]);
        if (interval < 1) {
            printf("错误: 间隔时间必须大于等于1秒\n");
            return 1;
        }
        
        cmd.type = CMD_UPDATE_INTERVAL;
        cmd.data.interval = (unsigned int)interval;
        
        printf("设置定时间隔为%d秒\n", interval);
        return send_command(&cmd);
    }
    
    /* 处理exit命令 */
    if (strcmp(argv[1], "exit") == 0) {
        cmd.type = CMD_EXIT;
        
        printf("发送退出命令\n");
        return send_command(&cmd);
    }
    
    /* 未知命令 */
    printf("错误: 未知命令 '%s'\n", argv[1]);
    print_usage(argv[0]);
    return 1;
} 