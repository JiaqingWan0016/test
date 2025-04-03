# 编译器和标志
CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
LDFLAGS = -L./lib -lse_vpn

# 目标文件
SRCS = src/main.c src/config.c src/netlink.c src/timer.c src/shm.c src/log.c
OBJS = $(SRCS:.c=.o)
TARGET = linkd

# 客户端工具
CLIENT_SRC = src/linkd_client.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
CLIENT_TARGET = linkd_client

# 测试相关
TEST_SRCS = $(wildcard tests/*.c)
TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_LIBS = -lcheck

# 主要目标
all: $(TARGET) $(CLIENT_TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(CLIENT_TARGET): $(CLIENT_OBJ)
	$(CC) $(CLIENT_OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -I./include -c $< -o $@

# 测试目标
test: $(TEST_OBJS)
	$(CC) $(TEST_OBJS) -o test_runner $(TEST_LIBS)

# 安装目标
install: $(TARGET) $(CLIENT_TARGET)
	mkdir -p /tos/conf
	touch /tos/conf/linkd.conf
	cp $(TARGET) /usr/local/bin/
	cp $(CLIENT_TARGET) /usr/local/bin/

# 清理目标
clean:
	rm -f $(OBJS) $(CLIENT_OBJ) $(TEST_OBJS) $(TARGET) $(CLIENT_TARGET) test_runner

.PHONY: all test install clean 