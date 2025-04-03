# 定义可执行文件
bin_PROGRAMS = linkd linkd_client

# linkd主程序
linkd_SOURCES = main.c config.c log.c network.c timer.c socket.c
linkd_CFLAGS = -std=c99 -Wall -Wextra -I$(top_srcdir)/include
linkd_LDADD = -lpthread

# linkd客户端程序
linkd_client_SOURCES = linkd_client.c
linkd_client_CFLAGS = -std=c99 -Wall -Wextra
 