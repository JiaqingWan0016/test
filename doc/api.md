# LINKD API文档

## 模块函数列表

### 配置管理模块 (config.c)

#### 函数列表

| 函数名 | 功能 | 参数 | 返回值 | 说明 |
|--------|------|------|--------|------|
| init_config | 初始化配置 | 无 | int | 成功返回0，失败返回-1 |
| cleanup_config | 清理配置 | 无 | void | 无 |
| get_config_item | 获取配置项 | const char *if_name | const IFBINDCONF_NAME* | 成功返回配置项指针，失败返回NULL |

### 网络事件处理模块 (netlink.c)

#### 函数列表

| 函数名 | 功能 | 参数 | 返回值 | 说明 |
|--------|------|------|--------|------|
| init_netlink | 初始化netlink | 无 | int | 成功返回0，失败返回-1 |
| handle_netlink_event | 处理netlink事件 | 无 | void | 无 |

### 定时器模块 (timer.c)

#### 函数列表

| 函数名 | 功能 | 参数 | 返回值 | 说明 |
|--------|------|------|--------|------|
| init_timer | 初始化定时器 | 无 | int | 成功返回0，失败返回-1 |
| cleanup_timer | 清理定时器 | 无 | void | 无 |

### 共享内存模块 (shm.c)

#### 函数列表

| 函数名 | 功能 | 参数 | 返回值 | 说明 |
|--------|------|------|--------|------|
| init_shared_memory | 初始化共享内存 | 无 | int | 成功返回0，失败返回-1 |
| cleanup_shared_memory | 清理共享内存 | 无 | void | 无 |
| read_shared_memory | 读取共享内存 | struct linkinfo *info | int | 成功返回0，失败返回-1 |
| update_shared_memory | 更新共享内存 | const struct linkinfo *info | int | 成功返回0，失败返回-1 |
| notify_vdcd_process | 通知vdcd进程 | 无 | int | 成功返回0，失败返回-1 |

### 日志管理模块 (log.c)

#### 函数列表

| 函数名 | 功能 | 参数 | 返回值 | 说明 |
|--------|------|------|--------|------|
| init_log | 初始化日志 | 无 | int | 成功返回0，失败返回-1 |
| cleanup_log | 清理日志 | 无 | void | 无 |
| log_write | 写入日志 | int level, const char *fmt, ... | void | 无 |

### 接口同步模块 (if_sync.c)

#### 函数列表

| 函数名 | 功能 | 参数 | 返回值 | 说明 |
|--------|------|------|--------|------|
| sync_interface_state | 同步接口状态 | const char *binding_if_name | int | 成功返回0，失败返回-1 |

### 接口地址管理模块 (if_addr.c)

#### 函数列表

| 函数名 | 功能 | 参数 | 返回值 | 说明 |
|--------|------|------|--------|------|
| get_if_ipv4_addr | 获取接口IPv4地址 | const char *if_name, struct if_ipv4_addr *addr, uint32_t specified_addr | int | 成功返回0，失败返回-1 |
| get_if_ipv6_addr | 获取接口IPv6地址 | const char *if_name, struct if_ipv6_addr *addr, const uint32_t *specified_addr | int | 成功返回0，失败返回-1 |

## 数据结构定义

### 链路信息结构体
```c
struct linkinfo {
    char virtualinterface[PHYSICALIF_LEN];  /* IPsec接口名称 */
    char physical[PHYSICALIF_LEN];          /* 绑定接口名称 */
    uint32_t interfaceip;                   /* IPv4地址 */
    uint32_t netmask;                       /* IPv4掩码 */
    uint32_t ipv6[4];                       /* IPv6地址 */
    uint32_t mtu;                           /* MTU值 */
    uint32_t linkstate;                     /* 链路状态 */
    uint32_t linkpriority;                  /* 链路优先级 */
};
```

### IPv4地址结构体
```c
struct if_ipv4_addr {
    uint32_t addr;      /* IPv4地址 */
    uint32_t netmask;   /* IPv4掩码 */
};
```

### IPv6地址结构体
```c
struct if_ipv6_addr {
    uint32_t addr[4];   /* IPv6地址 */
};
```

## 错误码定义

| 错误码 | 说明 |
|--------|------|
| 0 | 成功 |
| -1 | 失败 |

## 使用示例

### 初始化系统
```c
int main(void)
{
    /* 初始化日志 */
    if (init_log() < 0) {
        return -1;
    }
    
    /* 初始化配置 */
    if (init_config() < 0) {
        cleanup_log();
        return -1;
    }
    
    /* 初始化共享内存 */
    if (init_shared_memory() < 0) {
        cleanup_config();
        cleanup_log();
        return -1;
    }
    
    /* 初始化netlink */
    if (init_netlink() < 0) {
        cleanup_shared_memory();
        cleanup_config();
        cleanup_log();
        return -1;
    }
    
    /* 初始化定时器 */
    if (init_timer() < 0) {
        cleanup_netlink();
        cleanup_shared_memory();
        cleanup_config();
        cleanup_log();
        return -1;
    }
    
    return 0;
}
```

### 同步接口状态
```c
/* 同步绑定接口状态到IPsec接口 */
sync_interface_state("eth0");