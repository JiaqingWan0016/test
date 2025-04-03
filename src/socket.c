/**
 * @file socket.c
 * @brief 本地套接字通信实现
 */

#include "../include/socket.h"
#include "../include/log.h"
#include "../include/timer.h"

/* 全局变量 */
static int g_socket_fd = -1;        /* 服务器套接字描述符 */
static int g_client_fd = -1;        /* 客户端套接字描述符 */

/**
 * @brief 初始化本地套接字服务
 * 
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int socket_init(void)
{
    struct sockaddr_un addr;
    
    LOG_INFO("Initializing local socket server");
    
    /* 创建UNIX域套接字 */
    g_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (g_socket_fd < 0) {
        LOG_ERROR("Failed to create socket: %s", strerror(errno));
        return ERROR;
    }
    
    /* 设置非阻塞模式 */
    int flags = fcntl(g_socket_fd, F_GETFL, 0);
    fcntl(g_socket_fd, F_SETFL, flags | O_NONBLOCK);
    
    /* 设置地址 */
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    
    /* 删除可能存在的旧套接字文件 */
    unlink(SOCKET_PATH);
    
    /* 绑定套接字 */
    if (bind(g_socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        LOG_ERROR("Failed to bind socket: %s", strerror(errno));
        close(g_socket_fd);
        g_socket_fd = -1;
        return ERROR;
    }
    
    /* 开始监听 */
    if (listen(g_socket_fd, 5) < 0) {
        LOG_ERROR("Failed to listen on socket: %s", strerror(errno));
        close(g_socket_fd);
        g_socket_fd = -1;
        unlink(SOCKET_PATH);
        return ERROR;
    }
    
    LOG_INFO("Local socket server started on %s", SOCKET_PATH);
    return SUCCESS;
}

/**
 * @brief 处理套接字命令
 * 
 * @return 成功返回SUCCESS，失败返回ERROR
 */
int socket_handle_command(void)
{
    struct command cmd;
    int ret;
    
    /* 检查是否有套接字描述符 */
    if (g_socket_fd < 0) {
        return ERROR;
    }
    
    /* 检查是否有活动的客户端连接 */
    if (g_client_fd < 0) {
        /* 尝试接受新连接 */
        g_client_fd = accept(g_socket_fd, NULL, NULL);
        if (g_client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                /* 非阻塞模式下没有连接请求 */
                return SUCCESS;
            }
            LOG_ERROR("Failed to accept connection: %s", strerror(errno));
            return ERROR;
        }
        
        /* 设置客户端套接字为非阻塞模式 */
        int flags = fcntl(g_client_fd, F_GETFL, 0);
        fcntl(g_client_fd, F_SETFL, flags | O_NONBLOCK);
        
        LOG_INFO("New client connection accepted, fd: %d", g_client_fd);
        return SUCCESS;
    }
    
    /* 从客户端接收命令 */
    ret = recv(g_client_fd, &cmd, sizeof(cmd), 0);
    if (ret <= 0) {
        if (ret == 0 || errno != EAGAIN) {
            /* 连接已关闭或发生错误 */
            LOG_INFO("Client connection closed, fd: %d", g_client_fd);
            close(g_client_fd);
            g_client_fd = -1;
        }
        return SUCCESS;
    }
    
    /* 处理命令 */
    switch (cmd.type) {
    case CMD_UPDATE_INTERVAL:
        LOG_INFO("Received command: UPDATE_INTERVAL, value: %u", cmd.data.interval);
        timer_update_interval(cmd.data.interval);
        break;
        
    case CMD_EXIT:
        LOG_INFO("Received command: EXIT");
        /* 发送成功响应 */
        {
            char response[] = "Command executed successfully";
            send(g_client_fd, response, sizeof(response), 0);
        }
        return ERROR;  /* 退出程序 */
        
    default:
        LOG_WARN("Unknown command type: %d", cmd.type);
        break;
    }
    
    /* 发送成功响应 */
    {
        char response[] = "Command executed successfully";
        send(g_client_fd, response, sizeof(response), 0);
    }
    
    return SUCCESS;
}

/**
 * @brief 检查是否有活动的套接字连接
 * 
 * @param fds 文件描述符集合
 * @return 有连接返回TRUE，无连接返回FALSE
 */
int socket_check_events(fd_set *fds)
{
    if (g_socket_fd < 0) {
        return FALSE;
    }
    
    if (FD_ISSET(g_socket_fd, fds)) {
        return TRUE;
    }
    
    if (g_client_fd >= 0 && FD_ISSET(g_client_fd, fds)) {
        return TRUE;
    }
    
    return FALSE;
}

/**
 * @brief 获取套接字文件描述符
 * 
 * @return 文件描述符
 */
int socket_get_fd(void)
{
    return g_socket_fd;
}

/**
 * @brief 清理套接字资源
 */
void socket_cleanup(void)
{
    if (g_client_fd >= 0) {
        close(g_client_fd);
        g_client_fd = -1;
    }
    
    if (g_socket_fd >= 0) {
        close(g_socket_fd);
        g_socket_fd = -1;
        unlink(SOCKET_PATH);
    }
    
    LOG_INFO("Socket resources cleaned up");
} 