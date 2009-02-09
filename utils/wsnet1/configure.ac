dnl -*-m4-*-
dnl Process this file with autoconf to produce a configure script.
dnl --------------------------------------------------------------
dnl configure.in for Worldsens architecture simulator
dnl --------------------------------------------------------------
AC_INIT([wsnet1],[0.5])
AC_CONFIG_SRCDIR(src/worldsens_wsnet.c)
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

HOSTTYPE="none"
NEEDLARGEFILES="no"

case "${host}" in
	i686-*-linux-* ) 
	# i686-pc-linux-gnu
	HOSTTYPE="Linux"
	CFLAGS="$CFLAGS -DLINUX"
	NEEDLARGEFILES="yes"
	EXTRALIBS="$EXTRALIBS -lm"
	;;

	x86_64-*-linux-* ) 
	# x86_64-unknown-linux-gnu
	HOSTTYPE="Linux"
	CFLAGS="$CFLAGS -DLINUX"
	NEEDLARGEFILES="yes"
	EXTRALIBS="$EXTRALIBS -lm"
	;;

	*-*-solaris* )
        # cimirly  sparc-sun-solaris2.9
        # clash    sparc-sun-solaris2.10
	HOSTTYPE="Solaris"
	CFLAGS="$CFLAGS -DSOLARIS"
	EXTRALIBS="$EXTRALIBS -lsocket -lnsl"
	;;

	*-*-cygwin )
	# xp i686-pc-cygwin
	HOSTTYPE="Cygwin"
	CFLAGS="$CFLAGS -DCYGWIN"
	NEEDLARGEFILES="yes"
	#if test "${worldsens_sum}" != "no" ; then
	#   EXTRALIBS="-lsocket -lnsl"
        #fi
	EXTRALIBS="$EXTRALIBS -lwsock32"
	if test "${enable_gui}" != "no" ; then
	   EXTRALIBS="$EXTRALIBS -lSDL" 
	fi
	;;

	*-*-mingw32msvc )
	# i586-pc-mingw32msvc
	HOSTTYPE="Mingw32"
	CFLAGS="$CFLAGS -DMINGW32"
	NEEDLARGEFILES="yes"
	EXTRALIBS="$EXTRALIBS -lwsock32"
	if test "${enable_gui}" != "no" ; then
	   EXTRALIBS="$EXTRALIBS -lSDL" 
	fi
	;;

	*-apple-darwin* )
        # powerpc-apple-darwin8.6.0 
	# i686-apple-darwin8.9.1
	HOSTTYPE="MacOSX"
	CFLAGS="$CFLAGS -DDARWIN -DMACOSX"
	NEEDLARGEFILES="yes"
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

dnl -pipe
dnl =====
CFLAGS_save="${CFLAGS}"
AC_CACHE_CHECK([if \$CC accepts -pipe],
    [ac_cv_c_pipe],
    [CFLAGS="-pipe ${CFLAGS_save}"
     AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],[ac_cv_c_pipe=yes],[ac_cv_c_pipe=no])])
CFLAGS="${CFLAGS_save}"
if test "${ac_cv_c_pipe}" != "no"; then
    CFLAGS="-pipe ${CFLAGS}"
fi

dnl Debug mode
dnl ==========
CFLAGS_save="${CFLAGS}"
AC_ARG_ENABLE([debug],AC_HELP_STRING([--enable-debug],[debug mode (default disabled)]))
if test "${enable_debug}" = "yes"; then
    CFLAGS="$CFLAGS -g -DDEBUG"
    AC_DEFINE([DEBUG],[1],[WSNet internals debug tools])
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
dnl Host specific workarounds
dnl --------------------------------------------------------------


HOSTNAME=`hostname`
case "$HOSTNAME" in
	"pistols" )
	# pentium-m 
	# CFLAGS="$CFLAGS -march=pentium-m"
	# CFLAGS="$CFLAGS -march=pentium-m -mfpmath=sse"
	;;
	"clash" )
	# Ultra Sparc 2
	CFLAGS="$CFLAGS -mcpu=v9"
	;;
esac

if test "${NEEDLARGEFILES}" != "no"; then
	AC_DEFINE([_LARGEFILE_SOURCE], [1],[Defines LFS])
	AC_DEFINE([_FILE_OFFSET_BITS],[64],[Defines LFS])
fi


AC_DEFINE_UNQUOTED(HOSTTYPE,    $HOSTTYPE, [Host type])
AC_DEFINE_UNQUOTED(HOSTTYPESTR,"$HOSTTYPE",[Host type name])

AM_CONDITIONAL([BUILD_LINUX],   [test "$HOSTTYPE" = "Linux"])
AM_CONDITIONAL([BUILD_SOLARIS], [test "$HOSTTYPE" = "Solaris"])
AM_CONDITIONAL([BUILD_MACOSX],  [test "$HOSTTYPE" = "MacOSX"])
AM_CONDITIONAL([BUILD_MINGW32], [test "$HOSTTYPE" = "Mingw32"])
AM_CONDITIONAL([BUILD_CYGWIN],  [test "$HOSTTYPE" = "Cygwin"])

dnl Wextra && Werror
dnl ================
CFLAGS_save="${CFLAGS}"
AC_ARG_ENABLE([waou],AC_HELP_STRING([--enable-waou],[Wextra Werror mode (default disabled)]))
if test "${enable_waou}" = "yes"; then
    if test "${HOSTTYPE}" != "Cygwin" ; then
       CFLAGS="$CFLAGS -Wextra -Werror"
       waou_sum="yes"
    else
       waou_sum="no"
    fi
else
    waou_sum="no"
fi

dnl --------------------------------------------------------------
dnl Output
dnl --------------------------------------------------------------
dnl AC_DEFINE_UNQUOTED(WORLDSENS,"1",[Host type name])

CFLAGS="$CFLAGS -DWORLDSENS"

AC_SUBST(EXTRALIBS)

AC_CONFIG_FILES(
models/Makefile
models/antenna/Makefile
models/interference/Makefile
models/mobility/Makefile
models/modulation/Makefile
models/propagation/Makefile
src/Makefile
Makefile)
AC_OUTPUT

