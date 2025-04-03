/**
 * @file config.c
 * @brief 配置文件处理模块实现
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
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include "common.h"
#include "config.h"
#include "log.h"
#include <ctype.h>
#include "linkd.h"

/* 全局配置结构 */
static struct interface_config g_config;

/* 配置文件读取失败后的重试时间间隔（秒） */
#define CONFIG_RETRY_INTERVAL 10

/**
 * @brief 初始化配置系统
 * 
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int config_init(void)
{
    LOG_INFO("Initializing configuration system");
    
    /* 初始化配置结构 */
    memset(&g_config, 0, sizeof(g_config));
    g_config.interface_count = 0;
    
    /* 加载配置 */
    int ret = config_load();
    
    return ret;
}

/**
 * @brief 释放接口内存
 */
static void free_interfaces(void)
{
    for (int i = 0; i < g_config.interface_count; i++) {
        if (g_config.interfaces[i]) {
            free(g_config.interfaces[i]);
            g_config.interfaces[i] = NULL;
        }
    }
    g_config.interface_count = 0;
}

/**
 * @brief 加载配置文件
 * 
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int config_load(void)
{
    FILE *fp;
    char line[MAX_LINE_LENGTH];
    struct stat st;
    int retry_count = 0;
    const int max_retry = 3;
    
    LOG_INFO("Loading configuration from %s", CONFIG_FILE_PATH);
    
    /* 尝试打开配置文件，最多重试3次 */
    while (retry_count < max_retry) {
        fp = fopen(CONFIG_FILE_PATH, "r");
        if (fp)
            break;
            
        LOG_ERROR("Failed to open config file %s: %s (retry %d/%d)", 
                 CONFIG_FILE_PATH, strerror(errno), retry_count + 1, max_retry);
                 
        /* 如果重试次数未达到最大值，则等待后重试 */
        if (++retry_count < max_retry) {
            sleep(CONFIG_RETRY_INTERVAL);
        } else {
            LOG_ERROR("Max retry reached, giving up");
            return ERROR;
        }
    }
    
    /* 获取文件最后修改时间 */
    if (stat(CONFIG_FILE_PATH, &st) == 0) {
        g_config.last_modified = st.st_mtime;
    } else {
        LOG_WARN("Failed to get file stats: %s", strerror(errno));
    }
    
    /* 释放之前的接口内存 */
    free_interfaces();
    
    /* 读取配置文件的第一行，包含接口列表 */
    if (fgets(line, sizeof(line), fp) != NULL) {
        char *token, *saveptr;
        
        /* 去除行尾的换行符 */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        
        /* 解析空格分隔的接口名称 */
        for (token = strtok_r(line, " \t", &saveptr);
             token != NULL && g_config.interface_count < MAX_INTERFACES;
             token = strtok_r(NULL, " \t", &saveptr)) {
            
            /* 跳过空字符串 */
            if (*token == '\0')
                continue;
                
            /* 分配内存并复制接口名称 */
            g_config.interfaces[g_config.interface_count] = strdup(token);
            if (g_config.interfaces[g_config.interface_count] == NULL) {
                LOG_ERROR("Failed to allocate memory for interface name");
                continue;
            }
            
            LOG_INFO("Added interface: %s", g_config.interfaces[g_config.interface_count]);
            g_config.interface_count++;
        }
    }
    
    fclose(fp);
    
    if (g_config.interface_count == 0) {
        LOG_WARN("No interfaces found in config file");
    } else {
        LOG_INFO("Loaded %d interfaces from config file", g_config.interface_count);
    }
    
    return SUCCESS;
}

/**
 * @brief 重新加载配置文件
 * 
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int config_reload(void)
{
    LOG_INFO("Reloading configuration");
    return config_load();
}

/**
 * @brief 检查配置文件是否更新
 * 
 * @return 有更新返回TRUE，无更新返回FALSE
 */
int config_check_update(void)
{
    struct stat st;
    
    /* 检查文件是否存在并获取最后修改时间 */
    if (stat(CONFIG_FILE_PATH, &st) != 0) {
        LOG_ERROR("Failed to stat config file: %s", strerror(errno));
        return FALSE;
    }
    
    /* 检查最后修改时间是否变化 */
    if (st.st_mtime > g_config.last_modified) {
        LOG_INFO("Config file has been modified, last_modified: %ld, current: %ld",
                g_config.last_modified, st.st_mtime);
        return TRUE;
    }
    
    return FALSE;
}

/**
 * @brief 获取接口配置指针
 * 
 * @return 接口配置结构指针
 */
struct interface_config *config_get_interfaces(void)
{
    return &g_config;
}

/**
 * @brief 清理配置系统资源
 */
void config_cleanup(void)
{
    free_interfaces();
    LOG_INFO("Configuration system cleaned up");
}

/* 加载配置文件 */
int load_config(const char *conf_path, IFBIND_CONF_HEAD *head, IFBINDCONF_NAME **items)
{
    FILE *fp;
    size_t read_size;
    
    if (!conf_path || !head || !items) {
        log_write(LOG_LEVEL_ERROR, "Invalid parameters");
        return -1;
    }
    
    /* 打开配置文件 */
    fp = fopen(conf_path, "rb");
    if (!fp) {
        log_write(LOG_LEVEL_ERROR, "Failed to open config file: %s", conf_path);
        return -1;
    }
    
    /* 读取文件头部 */
    read_size = fread(head, sizeof(IFBIND_CONF_HEAD), 1, fp);
    if (read_size != 1) {
        log_write(LOG_LEVEL_ERROR, "Failed to read config header");
        fclose(fp);
        return -1;
    }
    
    /* 验证魔数 */
    if (memcmp(head->magic, "IFBD", 4) != 0) {
        log_write(LOG_LEVEL_ERROR, "Invalid config file magic");
        fclose(fp);
        return -1;
    }
    
    /* 验证配置项数量 */
    if (head->item_num > MAX_IPSEC_INTERFACES) {
        log_write(LOG_LEVEL_ERROR, "Too many config items: %lu", head->item_num);
        fclose(fp);
        return -1;
    }
    
    /* 分配配置项数组 */
    *items = malloc(sizeof(IFBINDCONF_NAME) * head->item_num);
    if (!*items) {
        log_write(LOG_LEVEL_ERROR, "Failed to allocate config items");
        fclose(fp);
        return -1;
    }
    
    /* 读取配置项 */
    read_size = fread(*items, sizeof(IFBINDCONF_NAME), head->item_num, fp);
    if (read_size != head->item_num) {
        log_write(LOG_LEVEL_ERROR, "Failed to read config items");
        free(*items);
        fclose(fp);
        return -1;
    }
    
    /* 验证配置项 */
    if (!validate_config(head, *items)) {
        log_write(LOG_LEVEL_ERROR, "Invalid config items");
        free(*items);
        fclose(fp);
        return -1;
    }
    
    fclose(fp);
    log_write(LOG_LEVEL_INFO, "Successfully loaded config file");
    return 0;
}

/* 重新加载配置文件 */
int reload_config(void)
{
    IFBIND_CONF_HEAD new_head;
    IFBINDCONF_NAME *new_items;
    
    /* 加载新配置 */
    if (load_config(IFBIND_CONF_PATH, &new_head, &new_items) < 0) {
        return -1;
    }
    
    /* 更新全局配置 */
    memcpy(&g_ctx.conf_head, &new_head, sizeof(IFBIND_CONF_HEAD));
    free(g_ctx.conf_items);
    g_ctx.conf_items = new_items;
    
    log_write(LOG_LEVEL_INFO, "Successfully reloaded config file");
    return 0;
}

/* 验证配置 */
int validate_config(const IFBIND_CONF_HEAD *head, const IFBINDCONF_NAME *items)
{
    int i;
    
    /* 验证魔数 */
    if (memcmp(head->magic, "IFBD", 4) != 0) {
        return 0;
    }
    
    /* 验证配置项数量 */
    if (head->item_num > MAX_IPSEC_INTERFACES) {
        return 0;
    }
    
    /* 验证每个配置项 */
    for (i = 0; i < head->item_num; i++) {
        const IFBINDCONF_NAME *item = &items[i];
        
        /* 验证ipsec接口名称 */
        if (strncmp(item->if_name, "ipsec", 5) != 0) {
            return 0;
        }
        
        /* 验证ipsec接口序号 */
        int index = atoi(item->if_name + 5);
        if (index < 0 || index >= MAX_IPSEC_INTERFACES) {
            return 0;
        }
        
        /* 验证绑定接口名称 */
        if (strlen(item->ibc.dev) == 0 || strlen(item->ibc.dev) >= IFNAMSIZ) {
            return 0;
        }
        
        /* 验证链路优先级 */
        if (item->ibc.linkpriority >= MAX_IPSEC_INTERFACES) {
            return 0;
        }
    }
    
    return 1;
} 