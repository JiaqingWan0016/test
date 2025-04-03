/**
 * @file timer.c
 * @brief 定时器模块实现
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
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#include "common.h"
#include "timer.h"
#include "log.h"
#include "network.h"
#include "linkd.h"

/* 全局定时器配置 */
static struct timer_config g_timer;

/* 定时器相关变量 */
static struct {
    int interval;
    time_t last_run;
    int running;
} g_timer_local;

/**
 * @brief 初始化定时器模块
 * 
 * @param default_interval 默认定时间隔（秒）
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int timer_init(unsigned int default_interval)
{
    LOG_INFO("Initializing timer module with interval: %u seconds", default_interval);
    
    /* 设置默认值 */
    g_timer_local.interval = default_interval;
    g_timer_local.last_run = time(NULL);
    g_timer_local.running = 1;
    
    LOG_INFO("Successfully initialized timer with interval %d seconds", default_interval);
    return SUCCESS;
}

/**
 * @brief 更新定时间隔
 * 
 * @param new_interval 新的定时间隔（秒）
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int timer_update_interval(unsigned int new_interval)
{
    LOG_INFO("Updating timer interval: %u -> %u seconds", g_timer_local.interval, new_interval);
    
    /* 更新间隔 */
    g_timer_local.interval = new_interval;
    
    LOG_INFO("Timer interval updated to %d seconds", new_interval);
    return SUCCESS;
}

/**
 * @brief 执行定时任务
 * 
 * @return 成功返回SUCCESS，失败返回ERROR
 */
static int run_timer_task(void)
{
    LOG_INFO("Running timer tasks");
    
    /* 任务1：打印所有接口状态 */
    network_print_status();
    
    /* 任务2：打印当前系统时间 */
    time_t now;
    struct tm tm_now;
    char time_str[64];
    
    time(&now);
    localtime_r(&now, &tm_now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_now);
    
    LOG_INFO("Current system time: %s", time_str);
    
    return SUCCESS;
}

/**
 * @brief 检查并执行定时任务
 * 
 * @return 执行了任务返回TRUE，没有执行任务返回FALSE
 */
int timer_check_and_run(void)
{
    time_t current_time = time(NULL);
    
    if (current_time - g_timer_local.last_run >= g_timer_local.interval) {
        timer_task_handler(NULL);
        g_timer_local.last_run = current_time;
        return TRUE;
    }
    
    return FALSE;
}

/**
 * @brief 获取当前定时间隔
 * 
 * @return 当前定时间隔（秒）
 */
unsigned int timer_get_interval(void)
{
    return g_timer_local.interval;
}

/**
 * @brief 获取下次运行时间
 * 
 * @return 下次运行时间
 */
time_t timer_get_next_run(void)
{
    return g_timer_local.last_run + g_timer_local.interval;
}

/**
 * @brief 清理定时器模块资源
 */
void timer_cleanup(void)
{
    LOG_INFO("Timer module cleaned up");
}

/* 定时任务处理函数 */
void timer_task_handler(void *arg)
{
    int i;
    
    /* 检查是否需要重新加载配置 */
    if (reload_config() < 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to reload config");
        return;
    }
    
    /* 遍历所有ipsec接口 */
    for (i = 0; i < g_ctx.conf_head.item_num; i++) {
        const IFBINDCONF_NAME *item = &g_ctx.conf_items[i];
        
        /* 同步接口状态 */
        if (sync_interface_state(item->ibc.dev) < 0) {
            log_write(LOG_LEVEL_ERROR, "Failed to sync interface state for %s", item->ibc.dev);
        }
    }
} 