/**
 * @file config.h
 * @brief 配置文件处理
 */
#ifndef _CONFIG_H
#define _CONFIG_H

#include "common.h"

/**
 * @brief 接口配置结构
 */
struct interface_config {
    char *interfaces[MAX_INTERFACES];  /* 接口名称数组 */
    int interface_count;               /* 接口数量 */
    time_t last_modified;              /* 配置文件最后修改时间 */
};

/**
 * @brief 初始化配置系统
 * 
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int config_init(void);

/**
 * @brief 加载配置文件
 * 
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int config_load(void);

/**
 * @brief 重新加载配置文件
 * 
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int config_reload(void);

/**
 * @brief 检查配置文件是否更新
 * 
 * @return 有更新返回TRUE，无更新返回FALSE
 */
int config_check_update(void);

/**
 * @brief 获取接口配置指针
 * 
 * @return 接口配置结构指针
 */
struct interface_config *config_get_interfaces(void);

/**
 * @brief 清理配置系统资源
 */
void config_cleanup(void);

#endif /* _CONFIG_H */ 