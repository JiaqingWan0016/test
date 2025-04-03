/**
 * @file log.c
 * @brief 日志系统实现
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
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#else
#include <stdarg.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#else
#include <sys/stat.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include "common.h"
#include "log.h"
#include "linkd.h"

/* 日志级别名称 */
static const char *log_level_names[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR"
};

/* 日志文件大小限制（20MB） */
#define MAX_LOG_SIZE (20 * 1024 * 1024)

/* 初始化日志系统 */
int init_log(const char *log_path, int level)
{
    if (!log_path) {
        fprintf(stderr, "Invalid log path\n");
        return -1;
    }
    
    /* 设置日志级别 */
    g_ctx.log_level = level;
    
    /* 打开日志文件 */
    g_ctx.log_fp = fopen(log_path, "a");
    if (!g_ctx.log_fp) {
        fprintf(stderr, "Failed to open log file: %s\n", log_path);
        return -1;
    }
    
    /* 设置文件权限 */
    chmod(log_path, 0644);
    
    log_write(LOG_LEVEL_INFO, "Log system initialized");
    return 0;
}

/* 日志轮转 */
static void log_rotate(void)
{
    char old_path[PATH_MAX];
    char new_path[PATH_MAX];
    struct stat st;
    
    if (!g_ctx.log_fp) {
        return;
    }
    
    /* 获取当前日志文件大小 */
    if (fstat(fileno(g_ctx.log_fp), &st) < 0) {
        return;
    }
    
    /* 如果文件大小超过限制，进行轮转 */
    if (st.st_size >= MAX_LOG_SIZE) {
        /* 关闭当前日志文件 */
        fclose(g_ctx.log_fp);
        
        /* 重命名当前日志文件 */
        snprintf(old_path, sizeof(old_path), "%s", "/tmp/.linkd_runlog");
        snprintf(new_path, sizeof(new_path), "%s.old", old_path);
        
        if (rename(old_path, new_path) < 0) {
            /* 如果重命名失败，尝试删除旧文件 */
            unlink(new_path);
            rename(old_path, new_path);
        }
        
        /* 重新打开日志文件 */
        g_ctx.log_fp = fopen(old_path, "a");
        if (g_ctx.log_fp) {
            chmod(old_path, 0644);
        }
    }
}

/* 写入日志 */
void log_write(int level, const char *fmt, ...)
{
    time_t now;
    struct tm *tm;
    char time_str[32];
    va_list ap;
    
    /* 检查日志级别 */
    if (level < g_ctx.log_level) {
        return;
    }
    
    /* 获取当前时间 */
    time(&now);
    tm = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm);
    
    /* 写入日志文件 */
    if (g_ctx.log_fp) {
        fprintf(g_ctx.log_fp, "[%s] [%s] ", time_str, log_level_names[level]);
        va_start(ap, fmt);
        vfprintf(g_ctx.log_fp, fmt, ap);
        va_end(ap);
        fprintf(g_ctx.log_fp, "\n");
        fflush(g_ctx.log_fp);
        
        /* 检查是否需要轮转 */
        log_rotate();
    }
    
    /* 写入标准输出 */
    if (!g_ctx.daemon_mode) {
        fprintf(stdout, "[%s] [%s] ", time_str, log_level_names[level]);
        va_start(ap, fmt);
        vfprintf(stdout, fmt, ap);
        va_end(ap);
        fprintf(stdout, "\n");
        fflush(stdout);
    }
}

/**
 * @brief 清理日志系统资源
 */
void log_cleanup(void)
{
    if (g_ctx.log_fp) {
        fclose(g_ctx.log_fp);
        g_ctx.log_fp = NULL;
    }
    LOG_INFO("Log system cleaned up");
} 