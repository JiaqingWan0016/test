# LINKD 架构设计文档

## 系统架构
LINKD采用模块化设计，主要包含以下核心模块：

1. 主程序模块（main.c）
   - 程序入口
   - 模块初始化
   - 主循环控制

2. 配置管理模块（config.c）
   - 配置文件解析
   - 配置信息管理
   - 动态配置更新

3. Netlink事件处理模块（netlink.c）
   - Netlink socket管理
   - 网络事件监听
   - 事件分发处理

4. 定时任务模块（timer.c）
   - 定时器管理
   - 定期检查任务
   - 任务调度

5. 共享内存模块（shm.c）
   - 共享内存创建
   - 数据读写操作
   - 进程间通信

6. 日志管理模块（log.c）
   - 日志级别控制
   - 日志文件管理
   - 日志轮转

7. 接口同步模块（if_sync.c）
   - 接口状态同步
   - IP地址同步
   - 接口操作执行

8. 接口地址管理模块（if_addr.c）
   - IPv4地址管理
   - IPv6地址管理
   - 地址获取和验证

## 模块依赖关系
```
main.c
  ├── config.c
  ├── netlink.c
  ├── timer.c
  ├── shm.c
  ├── log.c
  ├── if_sync.c
  └── if_addr.c
```

## 数据流
1. 配置加载流程
   ```
   配置文件 -> config.c -> 配置结构体 -> 其他模块
   ```

2. 事件处理流程
   ```
   Netlink事件 -> netlink.c -> if_sync.c -> if_addr.c -> 接口操作
   ```

3. 数据同步流程
   ```
   物理接口信息 -> if_addr.c -> if_sync.c -> shm.c -> vdcd进程
   ```

## 关键数据结构
1. 配置结构
```c
struct IFBINDCONF_NAME {
    char if_name[IFNAMSIZ];
    struct IFBINDCONF ibc;
};
```

2. 链路信息结构
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

## 核心算法
1. 地址选择算法
   - 优先使用配置的指定地址
   - IPv6地址排除链路本地地址
   - 按优先级选择地址

2. 接口同步算法
   - 检测接口状态变化
   - 比较地址信息变化
   - 执行必要的接口操作

3. 定时检查算法
   - 定期验证接口状态
   - 检查配置更新
   - 维护共享内存

## 错误处理机制
1. 错误类型
   - 配置错误
   - 系统调用错误
   - 网络错误
   - 资源错误

2. 错误恢复
   - 自动重试机制
   - 优雅降级
   - 资源清理

## 性能优化
1. 内存管理
   - 使用共享内存
   - 避免频繁分配
   - 及时释放资源

2. 事件处理
   - 批量处理事件
   - 异步处理机制
   - 优先级队列

3. 系统调用
   - 减少系统调用
   - 使用缓存机制
   - 批量操作优化

## 安全考虑
1. 权限控制
   - root权限检查
   - 文件权限管理
   - 进程权限控制

2. 数据安全
   - 敏感数据保护
   - 内存安全
   - 通信安全

3. 系统安全
   - 资源限制
   - 错误处理
   - 异常恢复 