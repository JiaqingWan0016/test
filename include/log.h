/**
 * @file log.h
 * @brief 日志系统
 */
#ifndef _LOG_H
#define _LOG_H

#include "common.h"

/* 日志级别 */
enum log_level {
    LOG_ERROR = 0,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG
};

/**
 * @brief 初始化日志系统
 * 
 * @param log_file 日志文件路径，若为NULL则仅输出到标准输出
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int log_init(const char *log_file);

/**
 * @brief 写入日志
 * 
 * @param level 日志级别
 * @param fmt 格式字符串
 * @param ... 可变参数
 */
void log_write(enum log_level level, const char *fmt, ...);

/**
 * @brief 检查并执行日志轮转
 * 
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int log_rotate(void);

/**
 * @brief 清理日志系统资源
 */
void log_cleanup(void);

/* 便捷宏定义 */
#define LOG_ERROR(fmt, ...) log_write(LOG_ERROR, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  log_write(LOG_WARN,  fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  log_write(LOG_INFO,  fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) log_write(LOG_DEBUG, fmt, ##__VA_ARGS__)

#endif /* _LOG_H */ 