# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([linkd], [1.0], [your-email@example.com])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])

# 添加libse_vpn库路径和头文件路径的配置选项
AC_ARG_WITH([libse-vpn],
    [AS_HELP_STRING([--with-libse-vpn=DIR],
    [specify the directory containing libse_vpn.so @<:@default=/usr/lib@:>@])],
    [LIBSE_VPN_DIR=$withval],
    [LIBSE_VPN_DIR=/usr/lib])

AC_ARG_WITH([libse-vpn-include],
    [AS_HELP_STRING([--with-libse-vpn-include=DIR],
    [specify the directory containing libse_vpn headers @<:@default=/usr/include@:>@])],
    [LIBSE_VPN_INCLUDE=$withval],
    [LIBSE_VPN_INCLUDE=/usr/include])

# 检查libse_vpn库
AC_CHECK_LIB([se_vpn], [init_shared_memory],
    [have_libse_vpn=yes],
    [have_libse_vpn=no],
    [-L$LIBSE_VPN_DIR])

if test "x$have_libse_vpn" = "xyes"; then
    AC_DEFINE([HAVE_LIBSE_VPN], [1], [Define if you have libse_vpn])
    LIBSE_VPN_LIBS="-L$LIBSE_VPN_DIR -lse_vpn"
    LIBSE_VPN_CFLAGS="-I$LIBSE_VPN_INCLUDE"
else
    AC_MSG_ERROR([libse_vpn library not found. Please specify the correct path using --with-libse-vpn])
fi

# 检查check库
PKG_CHECK_MODULES([CHECK], [check >= 0.10.0],
    [have_check=yes],
    [have_check=no])

if test "x$have_check" = "xyes"; then
    AC_DEFINE([HAVE_CHECK], [1], [Define if you have check])
    AC_SUBST([CHECK_CFLAGS])
    AC_SUBST([CHECK_LIBS])
fi

# 检查pthread库
AC_CHECK_LIB([pthread], [pthread_create],
    [have_pthread=yes],
    [have_pthread=no])

if test "x$have_pthread" = "xyes"; then
    AC_DEFINE([HAVE_PTHREAD], [1], [Define if you have pthread])
    PTHREAD_LIBS="-lpthread"
else
    AC_MSG_ERROR([pthread library not found])
fi

# 检查必要的头文件
AC_CHECK_HEADERS([sys/socket.h net/if.h linux/if.h linux/netlink.h linux/rtnetlink.h],
    [],
    [AC_MSG_ERROR([Required header files not found])])

# 检查必要的函数
AC_CHECK_FUNCS([strdup strndup],
    [],
    [AC_MSG_ERROR([Required functions not found])])

# 检查编译器特性
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# 设置编译标志
CFLAGS="$CFLAGS -Wall -Wextra -O2"
AC_SUBST([CFLAGS])

# 设置库标志
AC_SUBST([LIBSE_VPN_LIBS])
AC_SUBST([LIBSE_VPN_CFLAGS])
AC_SUBST([PTHREAD_LIBS])

AC_OUTPUT 