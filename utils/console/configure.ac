dnl -*-m4-*-
dnl Process this file with autoconf to produce a configure script.
dnl --------------------------------------------------------------
dnl configure.in for Worldsens architecture simulator
dnl --------------------------------------------------------------
AC_INIT([worldsens-console],[0.6])
AC_CONFIG_SRCDIR(src/main.c)
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
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

case "${host}" in
	*-*-linux-* ) 
	# i686-pc-linux-gnu
	# x86_64-unknown-linux-gnu
	# echo "Host is a linux system"
	HOSTTYPE="Linux"
	LINUX="yes"
	CFLAGS="$CFLAGS -DLINUX"
	EXTRALIBS=-lm
	;;

	*-*-solaris* )
        # cimirly  sparc-sun-solaris2.9
        # clash    sparc-sun-solaris2.10
	# echo "Host is solaris, libsocket and libnsl added"
	HOSTTYPE="Solaris"
	SOLARIS="yes"
	CFLAGS="$CFLAGS -DSOLARIS"
	EXTRALIBS="-lsocket -lnsl $EXTRALIBS"
	;;

	*-*-cygwin )
	HOSTTYPE="Cygwin"
	EXTRALIBS="-lsocket -lnsl -lSDL $EXTRALIBS"
	;;

	i586-pc-mingw32msvc )
	HOSTTYPE="Mingw32"
	EXTRALIBS="$EXTRALIBS"
	;;

	*-apple-darwin* )
        # powerpc-apple-darwin8.6.0 
	# echo "Host is Darwin/MacOSX"
	HOSTTYPE="MacOSX"
	MACOSX="yes"
	CFLAGS="$CFLAGS -DDARWIN -DMACOSX"
	;;
esac

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
if test "${enable_gui}" != "no" ; then
	SDL_VERSION=1.2.5
	AM_PATH_SDL($SDL_VERSION,   :,  AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])	)
	CFLAGS="$CFLAGS $SDL_CFLAGS"
	LDFLAGS="$LDFLAGS $SDL_LIBS"
fi

dnl --------------------------------------------------------------
dnl Output
dnl --------------------------------------------------------------
AC_SUBST(EXTRALIBS)

AC_CONFIG_FILES(
doc/Makefile
src/Makefile
Makefile)
AC_OUTPUT

