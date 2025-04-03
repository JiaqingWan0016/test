#include "linkd.h"

/* 初始化共享内存 */
int init_shared_memory(void)
{
    /* 创建共享内存 */
    if (createshm() < 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to create shared memory");
        return -1;
    }
    
    /* 分配共享内存结构 */
    g_ctx.shm = malloc(sizeof(struct sharememory));
    if (!g_ctx.shm) {
        log_write(LOG_LEVEL_ERROR, "Failed to allocate shared memory structure");
        deleteshm();
        return -1;
    }
    
    /* 初始化共享内存 */
    memset(g_ctx.shm, 0, sizeof(struct sharememory));
    g_ctx.shm->linkscount = g_ctx.conf_head.item_num;
    
    /* 写入共享内存 */
    if (writeshm(g_ctx.shm) < 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to write shared memory");
        free(g_ctx.shm);
        deleteshm();
        return -1;
    }
    
    log_write(LOG_LEVEL_INFO, "Successfully initialized shared memory");
    return 0;
}

/* 更新共享内存 */
int update_shared_memory(const struct linkinfo *info)
{
    if (!info || !g_ctx.shm) {
        log_write(LOG_LEVEL_ERROR, "Invalid parameters");
        return -1;
    }
    
    /* 检查链路优先级是否有效 */
    if (info->linkpriority >= MAX_IPSEC_INTERFACES) {
        log_write(LOG_LEVEL_ERROR, "Invalid link priority: %d", info->linkpriority);
        return -1;
    }
    
    /* 更新链路信息 */
    memcpy(&g_ctx.shm->link[info->linkpriority], info, sizeof(struct linkinfo));
    
    /* 写入共享内存 */
    if (writeshm(g_ctx.shm) < 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to write shared memory");
        return -1;
    }
    
    return 0;
}

/* 通知vdcd进程 */
int notify_vdcd_process(void)
{
    int retry_count = 0;
    int max_retries = 5;
    int retry_interval = 2;  /* 2秒 */
    
    while (retry_count < max_retries) {
        if (linkd_tosmsg_vdc() == 0) {
            return 0;
        }
        
        retry_count++;
        if (retry_count < max_retries) {
            log_write(LOG_LEVEL_WARN, "Failed to notify vdcd process, retrying in %d seconds...", retry_interval);
            sleep(retry_interval);
        }
    }
    
    log_write(LOG_LEVEL_ERROR, "Failed to notify vdcd process after %d retries", max_retries);
    return -1;
} 