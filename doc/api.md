# LINKD API文档

## 配置管理模块 (config.h)

### 函数列表
1. `int init_config(const char *config_file)`
   - 功能：初始化配置
   - 参数：
     - config_file: 配置文件路径
   - 返回值：成功返回0，失败返回-1

2. `void cleanup_config(void)`
   - 功能：清理配置资源
   - 参数：无
   - 返回值：无

3. `int reload_config(void)`
   - 功能：重新加载配置
   - 参数：无
   - 返回值：成功返回0，失败返回-1

## Netlink事件处理模块 (netlink.h)

### 函数列表
1. `int init_netlink(void)`
   - 功能：初始化netlink socket
   - 参数：无
   - 返回值：成功返回0，失败返回-1

2. `void cleanup_netlink(void)`
   - 功能：清理netlink资源
   - 参数：无
   - 返回值：无

3. `int handle_netlink_event(void)`
   - 功能：处理netlink事件
   - 参数：无
   - 返回值：成功返回0，失败返回-1

## 定时任务模块 (timer.h)

### 函数列表
1. `int init_timer(void)`
   - 功能：初始化定时器
   - 参数：无
   - 返回值：成功返回0，失败返回-1

2. `void cleanup_timer(void)`
   - 功能：清理定时器资源
   - 参数：无
   - 返回值：无

3. `int add_timer_task(void (*callback)(void), int interval)`
   - 功能：添加定时任务
   - 参数：
     - callback: 回调函数
     - interval: 间隔时间（秒）
   - 返回值：成功返回0，失败返回-1

## 共享内存模块 (shm.h)

### 函数列表
1. `int init_shared_memory(void)`
   - 功能：初始化共享内存
   - 参数：无
   - 返回值：成功返回0，失败返回-1

2. `void cleanup_shared_memory(void)`
   - 功能：清理共享内存资源
   - 参数：无
   - 返回值：无

3. `int update_shared_memory(const struct linkinfo *info)`
   - 功能：更新共享内存数据
   - 参数：
     - info: 链路信息结构体
   - 返回值：成功返回0，失败返回-1

4. `int read_shared_memory(struct linkinfo *info)`
   - 功能：读取共享内存数据
   - 参数：
     - info: 链路信息结构体指针
   - 返回值：成功返回0，失败返回-1

5. `int notify_vdcd_process(void)`
   - 功能：通知vdcd进程
   - 参数：无
   - 返回值：成功返回0，失败返回-1

## 日志管理模块 (log.h)

### 函数列表
1. `int init_log(const char *log_path, int log_level)`
   - 功能：初始化日志系统
   - 参数：
     - log_path: 日志文件路径
     - log_level: 日志级别
   - 返回值：成功返回0，失败返回-1

2. `void cleanup_log(void)`
   - 功能：清理日志资源
   - 参数：无
   - 返回值：无

3. `void log_write(int level, const char *format, ...)`
   - 功能：写入日志
   - 参数：
     - level: 日志级别
     - format: 格式化字符串
     - ...: 可变参数
   - 返回值：无

## 接口同步模块 (if_sync.h)

### 函数列表
1. `int sync_interface_state(const char *if_name)`
   - 功能：同步接口状态
   - 参数：
     - if_name: 接口名称
   - 返回值：成功返回0，失败返回-1

2. `int ipsec_if_down_up(const char *if_name)`
   - 功能：执行ipsec接口的down/up操作
   - 参数：
     - if_name: 接口名称
   - 返回值：成功返回0，失败返回-1

## 接口地址管理模块 (if_addr.h)

### 函数列表
1. `int get_if_ipv4_addr(const char *if_name, struct if_ipv4_addr *ipv4, uint32_t specified_addr)`
   - 功能：获取接口IPv4地址
   - 参数：
     - if_name: 接口名称
     - ipv4: IPv4地址结构体指针
     - specified_addr: 指定地址
   - 返回值：成功返回0，失败返回-1

2. `int get_if_ipv6_addr(const char *if_name, struct if_ipv6_addr *ipv6, const uint32_t *specified_addr)`
   - 功能：获取接口IPv6地址
   - 参数：
     - if_name: 接口名称
     - ipv6: IPv6地址结构体指针
     - specified_addr: 指定地址
   - 返回值：成功返回0，失败返回-1

## 数据结构定义

### 链路信息结构
```c
struct linkinfo {
    char virtualinterface[PHYSICALIF_LEN];
    char physical[PHYSICALIF_LEN];
    uint32_t interfaceip;
    uint32_t netmask;
    uint32_t ipv6[4];
    int linkstate;
    int mtu;
    int linkpriority;
};
```

### IPv4地址结构
```c
struct if_ipv4_addr {
    uint32_t addr;
    uint32_t netmask;
};
```

### IPv6地址结构
```c
struct if_ipv6_addr {
    uint32_t addr[4];
};
```

## 错误码定义
```c
#define SUCCESS 0
#define ERROR_INIT_FAILED -1
#define ERROR_INVALID_PARAM -2
#define ERROR_SYSTEM_CALL -3
#define ERROR_RESOURCE_BUSY -4
#define ERROR_TIMEOUT -5
```

## 使用示例

### 初始化示例
```c
int main(void) {
    /* 初始化日志 */
    if (init_log("/var/log/linkd.log", LOG_LEVEL_INFO) != 0) {
        return -1;
    }
    
    /* 初始化配置 */
    if (init_config("/etc/linkd/config.json") != 0) {
        cleanup_log();
        return -1;
    }
    
    /* 初始化其他模块 */
    if (init_netlink() != 0 || init_timer() != 0 || init_shared_memory() != 0) {
        cleanup_config();
        cleanup_log();
        return -1;
    }
    
    /* 主循环 */
    while (1) {
        handle_netlink_event();
        sleep(1);
    }
    
    return 0;
}
```

### 接口同步示例
```c
void sync_interface(const char *if_name) {
    struct linkinfo info;
    
    /* 同步接口状态 */
    if (sync_interface_state(if_name) != 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to sync interface %s", if_name);
        return;
    }
    
    /* 更新共享内存 */
    if (update_shared_memory(&info) != 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to update shared memory");
        return;
    }
    
    /* 通知vdcd进程 */
    if (notify_vdcd_process() != 0) {
        log_write(LOG_LEVEL_ERROR, "Failed to notify vdcd process");
        return;
    }
} 