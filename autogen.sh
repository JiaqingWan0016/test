#!/bin/sh

# 创建 m4 目录（如果不存在）
mkdir -p m4

# 创建配置目录并创建示例配置文件（如果不存在）
mkdir -p config
if [ ! -f config/linkd.conf ]; then
    echo "eth0 eth1" > config/linkd.conf
fi

# 运行 autotools 命令
autoreconf --force --install

echo
echo "现在您可以运行:"
echo "./configure"
echo "make"
echo 