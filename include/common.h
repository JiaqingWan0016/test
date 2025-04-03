/**
 * @file common.h
 * @brief 通用定义和宏
 */
#ifndef _COMMON_H
#define _COMMON_H

/* 包含自动生成的配置头文件 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

/* 检查系统头文件的存在性 */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

/* 路径长度定义 */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define MAX_INTERFACES 10
#define MAX_LINE_LENGTH 256
#define CONFIG_FILE_PATH "/tos/conf/linkd.conf"
#define LOG_FILE_PATH "/tmp/.linkd_runlog"
#define SOCKET_PATH "/tmp/linkd_socket"
#define MAX_LOG_SIZE (20 * 1024 * 1024) /* 20MB */

/* 状态码 */
#define SUCCESS 0
#define ERROR -1

/* 布尔值 */
#define TRUE 1
#define FALSE 0

/* 全局变量声明 */
extern int g_running;
extern int g_debug_mode;

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

#endif /* _COMMON_H */ 