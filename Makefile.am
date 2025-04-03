ACLOCAL_AMFLAGS = -I m4

# 包含的子目录
SUBDIRS = include src tests

# 额外的配置文件
sysconf_DATA = config/linkd.conf
sysconfdir = /tos/conf

# 额外分发的文件
EXTRA_DIST = config/linkd.conf \
             autogen.sh \
             README.md \
             INSTALL

# 清理时删除的额外内容
DISTCLEANFILES = config.h.in~ aclocal.m4 \
                 configure.ac~ Makefile.in \
                 compile config.guess config.sub \
                 depcomp install-sh ltmain.sh \
                 missing 

# 设置编译选项
AM_CFLAGS = $(LIBSE_VPN_CFLAGS) -Wall -Wextra -O2

# 设置链接选项
AM_LDFLAGS = $(LIBSE_VPN_LIBS) $(PTHREAD_LIBS)

# 主程序
bin_PROGRAMS = linkd

# 源文件
linkd_SOURCES = \
    src/main.c \
    src/config.c \
    src/netlink.c \
    src/timer.c \
    src/shm.c \
    src/log.c \
    src/if_sync.c \
    src/if_addr.c

# 头文件
include_HEADERS = \
    include/linkd.h

# 测试程序
if HAVE_CHECK
check_PROGRAMS = test_linkd
test_linkd_SOURCES = tests/test_linkd.c
test_linkd_CFLAGS = $(CHECK_CFLAGS)
test_linkd_LDADD = $(CHECK_LIBS)
endif

# 清理
CLEANFILES = *~ .*~ *.o *.lo *.la 