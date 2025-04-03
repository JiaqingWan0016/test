/**
 * @file timer.h
 * @brief 定时器模块
 */
#ifndef _TIMER_H
#define _TIMER_H

#include "common.h"

/**
 * @brief 定时器配置结构
 */
struct timer_config {
    unsigned int interval;      /* 定时间隔（秒） */
    time_t next_run;           /* 下次运行时间 */
};

/**
 * @brief 初始化定时器模块
 * 
 * @param default_interval 默认定时间隔（秒）
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int timer_init(unsigned int default_interval);

/**
 * @brief 更新定时间隔
 * 
 * @param new_interval 新的定时间隔（秒）
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int timer_update_interval(unsigned int new_interval);

/**
 * @brief 检查并执行定时任务
 * 
 * @return 执行了任务返回TRUE，没有执行任务返回FALSE
 */
int timer_check_and_run(void);

/**
 * @brief 获取当前定时间隔
 * 
 * @return 当前定时间隔（秒）
 */
unsigned int timer_get_interval(void);

/**
 * @brief 获取下次运行时间
 * 
 * @return 下次运行时间
 */
time_t timer_get_next_run(void);

/**
 * @brief 清理定时器模块资源
 */
void timer_cleanup(void);

#endif /* _TIMER_H */ 