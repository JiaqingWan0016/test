if HAVE_CHECK

# 测试程序
check_PROGRAMS = test_config

# 测试配置模块
test_config_SOURCES = test_config.c \
                      $(top_srcdir)/src/config.c \
                      $(top_srcdir)/src/log.c
test_config_CFLAGS = @CHECK_CFLAGS@ -I$(top_srcdir)/include
test_config_LDADD = @CHECK_LIBS@

# 测试目标
TESTS = $(check_PROGRAMS)

endif 