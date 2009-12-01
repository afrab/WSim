dnl -*-m4-*-
dnl Process this file with autoconf to produce a configure script.
dnl --------------------------------------------------------------
dnl configure.in for Worldsens architecture simulator
dnl --------------------------------------------------------------
AC_INIT([worldsens-tracer],[0.8svn])
AC_CONFIG_SRCDIR(src/main.c)
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([foreign])
AM_CONFIG_HEADER([config.h])

dnl --------------------------------------------------------------
dnl Checks for programs.
dnl --------------------------------------------------------------
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_RANLIB

dnl -------------------------------------------------------------- 
dnl Host specific workarounds
dnl --------------------------------------------------------------

dnl --------------------------------------------------------------
dnl Checks for compilation flags
dnl --------------------------------------------------------------

CFLAGS_save="${CFLAGS}"
AC_CACHE_CHECK([if \$CC accepts -Wall],
    [ac_cv_c_Wall],
    [CFLAGS="-Wall ${CFLAGS_save}"
     AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],[ac_cv_c_Wall=yes],[ac_cv_c_Wall=no])])
CFLAGS="${CFLAGS_save}"
if test "${ac_cv_c_Wall}" != "no"; then
    CFLAGS="-Wall ${CFLAGS}"
fi

dnl Wextra && Werror
dnl ================
CFLAGS_save="${CFLAGS}"
AC_ARG_ENABLE([waou],AC_HELP_STRING([--enable-waou],[Wextra Werror mode (default disabled)]))
if test "${enable_waou}" = "yes"; then
    if test "x${HOSTTYPE}" != "xCygwin" ; then
       CFLAGS="$CFLAGS -Wextra -Werror"
       waou_sum="yes"
    else
       waou_sum="no"
    fi
else
    waou_sum="no"
fi

dnl Debug mode
dnl ==========
CFLAGS_save="${CFLAGS}"
AC_ARG_ENABLE([debug],AC_HELP_STRING([--enable-debug],[debug mode (default disabled)]))
if test "${enable_debug}" = "yes"; then
    CFLAGS="$CFLAGS -g -DDEBUG"
    debug_sum="yes"
else
    debug_sum="no"
fi
AM_CONDITIONAL([BUILD_DEBUG], [test "${debug_sum}" = "yes"])

dnl --------------------------------------------------------------
dnl Checks for header files.
dnl --------------------------------------------------------------
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h unistd.h)

dnl --------------------------------------------------------------
dnl Checks for typedefs, structures, and compiler characteristics.
dnl --------------------------------------------------------------
AC_C_CONST
AC_TYPE_SIZE_T
AC_C_BIGENDIAN
AC_C_INLINE
AC_CHECK_SIZEOF(char, 1)
AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int *, 4)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(long long, 8)

dnl --------------------------------------------------------------
dnl Checks for library functions.
dnl --------------------------------------------------------------
AC_FUNC_MEMCMP

dnl --------------------------------------------------------------
dnl Checks for libraries.
dnl --------------------------------------------------------------

dnl --------------------------------------------------------------
dnl Output
dnl --------------------------------------------------------------
AC_SUBST(EXTRALIBS)

AC_CONFIG_FILES(
doc/Makefile
src/Makefile
Makefile)
AC_OUTPUT

