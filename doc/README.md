# LINKD

LINKD是一个Linux程序，用于同步IPsec接口和绑定接口信息。它通过netlink机制监控网络接口状态变化，并将绑定接口的IP地址、掩码和状态同步到对应的IPsec接口。

## 功能特性

1. 接口状态监控
   - 监控绑定接口的up/down状态
   - 监控IP地址变化
   - 监控MTU变化

2. 地址管理
   - 支持IPv4和IPv6地址
   - 自动过滤链路本地地址
   - 支持指定地址匹配

3. 接口同步
   - 同步绑定接口信息到IPsec接口
   - 执行IPsec接口的down/up操作
   - 更新IPsec接口状态

4. 配置管理
   - 从配置文件读取配置
   - 支持动态更新配置
   - 配置项验证

## 目录结构
```
.
├── src/                # 源代码目录
│   ├── main.c         # 主程序
│   ├── config.c       # 配置管理
│   ├── netlink.c      # Netlink事件处理
│   ├── timer.c        # 定时器管理
│   ├── shm.c          # 共享内存管理
│   ├── log.c          # 日志管理
│   ├── if_sync.c      # 接口同步
│   └── if_addr.c      # 接口地址管理
├── include/           # 头文件目录
│   ├── config.h       # 配置相关定义
│   ├── netlink.h      # Netlink相关定义
│   ├── timer.h        # 定时器相关定义
│   ├── shm.h          # 共享内存相关定义
│   ├── log.h          # 日志相关定义
│   ├── if_sync.h      # 接口同步相关定义
│   └── if_addr.h      # 接口地址相关定义
├── doc/               # 文档目录
│   ├── README.md      # 项目说明
│   ├── architecture.md # 架构设计
│   └── api.md         # API文档
└── Makefile          # 编译配置
```

## 编译说明

### 依赖项
- gcc
- make
- Linux系统头文件

### 编译命令
```bash
make
```

### 安装命令
```bash
make install
```

## 配置说明

配置文件格式为JSON，示例：
```json
{
    "bindings": [
        {
            "ipsec_if": "ipsec0",
            "binding_if": "eth0",
            "link_priority": 1,
            "ipv4": "192.168.1.100",
            "ipv6": "2001:db8::1"
        }
    ]
}
```

## 运行说明

### 启动程序
```bash
sudo ./linkd
```

### 查看日志
```bash
tail -f /var/log/linkd.log
```

## 注意事项

1. 权限要求
   - 需要root权限运行
   - 确保netlink支持
   - 确保IPsec配置正确

2. 系统要求
   - Linux内核版本 >= 2.6
   - 支持netlink机制
   - 支持IPsec功能

3. 配置要求
   - 正确配置IPsec接口
   - 正确配置绑定接口
   - 确保配置文件格式正确

## 常见问题

1. 权限问题
   - 确保使用root权限运行
   - 检查文件权限设置
   - 检查SELinux状态

2. 接口同步失败
   - 检查接口名称是否正确
   - 检查IPsec配置是否正确
   - 检查网络连接状态

3. 地址获取失败
   - 检查接口状态
   - 检查网络配置
   - 检查防火墙设置

## 维护说明

1. 日志管理
   - 定期检查日志文件大小
   - 配置日志轮转
   - 清理过期日志

2. 错误处理
   - 监控程序运行状态
   - 及时处理错误日志
   - 定期检查系统状态

3. 性能优化
   - 监控系统资源使用
   - 优化事件处理
   - 调整配置参数 