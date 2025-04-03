# LINKD

LINKD是一个用于同步ipsec接口与绑定接口信息的Linux程序。它通过监听网络接口事件和定时任务两种方式来保持ipsec接口与绑定接口的IP地址配置以及接口状态的一致性。

## 功能特点

- 支持动态加载/热更新配置文件
- 使用netlink机制监听网络接口事件
- 支持定时任务同步接口信息
- 支持守护进程模式运行
- 完整的日志记录和轮转机制
- 共享内存通信机制
- 错误恢复机制

## 系统要求

- Linux 4.14或更高版本
- GCC编译器
- C99标准支持
- libse_vpn库

## 编译安装

1. 克隆代码库：
```bash
git clone https://github.com/yourusername/linkd.git
cd linkd
```

2. 编译：
```bash
make
```

3. 安装：
```bash
sudo make install
```

## 使用方法

1. 启动程序：
```bash
# 前台运行
linkd

# 守护进程模式运行
linkd -d
```

2. 修改定时任务间隔：
```bash
# 通过本地套接字发送命令
echo "SET_INTERVAL 30" | nc -U /var/run/linkd.sock
```

## 配置文件

配置文件位于`/tos/conf/vpn/ifbind.conf`，包含以下内容：

- 文件头部：包含魔数、配置项数量等信息
- 配置项数组：每个配置项包含ipsec接口和绑定接口的信息

## 日志

- 日志文件：`/tmp/.linkd_runlog`
- 日志级别：DEBUG、INFO、WARN、ERROR
- 日志轮转：文件大小超过20MB时自动轮转

## 错误处理

程序实现了以下错误恢复机制：

1. 配置文件读取失败后的重试
2. 网络接口监听失败后的重新监听
3. 共享内存操作失败后的重试
4. vdcd进程通知失败后的重试

## 开发

### 代码风格

- 遵循Linux内核代码风格
- 使用C99标准
- 完整的注释和文档

### 测试

使用check-0.10.0进行单元测试：
```bash
make test
```

## 许可证

本项目采用MIT许可证。详见LICENSE文件。 