dnl -*-m4-*-
dnl Process this file with autoconf to produce a configure script.
dnl --------------------------------------------------------------
dnl configure.ac for Worldsens architecture simulator
dnl works only for automake 1.10 and above 
dnl lower versions will have problem with bzero prototype replacement
dnl --------------------------------------------------------------
AC_INIT([wsim],[0.99.1svn])
AC_CONFIG_SRCDIR(src/main.c)
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])

AC_DEFINE(WSIM_BUILD_ID,          36  , [build id]   )
AC_DEFINE(WSIM_BUILD_DATE, "03062008" , [build date] )
 
dnl --------------------------------------------------------------
dnl Checks for programs.
dnl --------------------------------------------------------------
AC_PROG_INSTALL
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_MAKE_SET
AC_PROG_RANLIB

dnl -------------------------------------------------------------- 
dnl Install directories
dnl --------------------------------------------------------------

EXTRALIBS=""

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

dnl Debug mode : enable debug code for wsim internals
dnl ============ 
CFLAGS_save="${CFLAGS}"
AC_ARG_ENABLE([debug],AS_HELP_STRING([--enable-debug],[debug mode (default disabled)]))
if test "${enable_debug}" = "yes"; then
    CFLAGS="$CFLAGS -g"
    AC_DEFINE([DEBUG],[1],[WSim internals debug tools])
    debug_sum="yes"
else
    debug_sum="no"
fi
AM_CONDITIONAL([BUILD_DEBUG], [test "${debug_sum}" = "yes"])

dnl Cross Debug mode : enable debug code for wsim target code
dnl ==================
CFLAGS_save="${CFLAGS}"
AC_ARG_ENABLE([xdebug],AS_HELP_STRING([--enable-xdebug],[target code debug (default enabled)]))
if test "${enable_xdebug}" != "no"; then
    xdebug_sum="yes"
    AC_DEFINE([XCODE_DEBUG],[1],[WSim target code debug tools])
else
    xdebug_sum="no"
fi
AM_CONDITIONAL([BUILD_XCODE_DEBUG], [test "${xdebug_sum}" != "no"])

dnl eTrace mode
dnl ===========
zlib_sum="system" 
AC_ARG_WITH([zlib-prefix], AS_HELP_STRING([--with-zlib-prefix=DIR],[zlib path]),
	    [CFLAGS="-I$withval/include $CFLAGS" 
	     LDFLAGS="-L$withval/lib $LDFLAGS" 
	     zlib_sum="yes" 
	     zlib_prefix=$withval],
	    [zlib_prefix=""])

CFLAGS_save="${CFLAGS}"
AC_ARG_ENABLE([etrace],AS_HELP_STRING([--enable-etrace],[etrace mode (default disabled)]))
if test "${enable_etrace}" = "yes"; then
    AC_CHECK_LIB(z, compress)
    AC_DEFINE([ETRACE],[1],[eSimu energy tracer])
    EXTRALIBS="${ETRACELIBPATH}/${ETRACELIBNAME} $EXTRALIBS"
    etrace_sum="yes"
else
    etrace_sum="no"
fi
AM_CONDITIONAL([BUILD_ETRACE], [test "${etrace_sum}" = "yes"])

dnl SDL ui mode
dnl ===========
AC_ARG_ENABLE([gui],AS_HELP_STRING([--enable-gui],[GUI for devices (default disabled)]))
if test "${enable_gui}" = "yes" ; then
    sdl_sum="yes"
    AC_DEFINE([GUI],[1],[Defines wether a GUI is available])
else
    sdl_sum="no"
fi
AM_CONDITIONAL([BUILD_GUI], [test "${sdl_sum}" = "yes"])

dnl Gprof mode
dnl ==========

dnl profile_sum="no"
dnl AC_ARG_ENABLE([profile],AS_HELP_STRING([--enable-profile],[profile mode (default disabled)]))
dnl if test "${enable_profile}" = "yes"; then
dnl     CFLAGS="$CFLAGS -g -pg -DPROFILE"
dnl     LDFLAGS="$LDFLAGS -pg"
dnl     profile_sum="yes"
dnl else
dnl     profile_sum="no"
dnl fi

dnl Tracer enable
dnl =============

dnl AC_ARG_ENABLE([tracer],AS_HELP_STRING([--enable-tracer],[Event tracer (default enabled)]))
dnl if test "${enable_tracer}" != "no" ; then
dnl     tracer_sum="yes"
dnl else
dnl     tracer_sum="no"
dnl fi
dnl AM_CONDITIONAL([BUILD_TRACER], [test "${tracer_sum}" = "yes"])

dnl --------------------------------------------------------------
dnl Devices flags
dnl --------------------------------------------------------------

dnl ptty enable
dnl ===========
AC_ARG_ENABLE([ptty],AS_HELP_STRING([--enable-ptty],[Pseudo TTY IO device (default disabled)]))
if test "${enable_ptty}" = "yes" ; then
   ptty_sum="yes"
   AC_DEFINE([PTTY],[1],[Defines wether libptty is used on the corrent configuration])
else
   ptty_sum="no"
fi
AM_CONDITIONAL([BUILD_PTTY], [test "${ptty_sum}" = "yes"])

dnl worldsens mode 
dnl ==============
worldsens_sum="no"
wsnet1_sum="no"
dnl wsnet1le_sum="no"
wsnet2_sum="no"

AC_ARG_ENABLE([wsnet1],AS_HELP_STRING([--enable-wsnet1],[WorldSens WSNet1 simulator (default disabled)]))
if test "${enable_wsnet1}" = "yes" ; then
   worldsens_sum="wsnet1"
   wsnet1_sum="yes"
fi 

dnl AC_ARG_ENABLE([wsnet1],AS_HELP_STRING([--enable-wsnet1le],[WorldSens WSNet1le simulator (default disabled)]))
dnl if test "${enable_wsnet1le}" = "yes" ; then
dnl    worldsens_sum="wsnet1le"
dnl    wsnet1le_sum="yes"
dnl fi 

AC_ARG_ENABLE([wsnet2],AS_HELP_STRING([--enable-wsnet2],[WorldSens WSNet2 simulator (default disabled)]))
if test "${enable_wsnet2}" = "yes" ; then
   worldsens_sum="wsnet2"
   wsnet2_sum="yes"
fi 

if test "${enable_wsnet1}" = "yes" -a "${enable_wsnet2}" = "yes" ; then 
   AC_MSG_NOTICE([== ------------------------------------------- ==])
   AC_MSG_NOTICE([== You need to select either wsnet1 or wsnet2, ==])
   AC_MSG_NOTICE([==    switching to WSNet2 mode                 ==])
   AC_MSG_NOTICE([== ------------------------------------------- ==])
   worldsens_sum="wsnet2"
   wsnet1_sum="no"
   wsnet2_sum="yes"
fi

AM_CONDITIONAL([BUILD_WORLDSENS],  [test "${worldsens_sum}" != "no"])
AM_CONDITIONAL([BUILD_WSNET1],     [test "${worldsens_sum}" = "wsnet1"])
dnl AM_CONDITIONAL([BUILD_WSNET1LE],   [test "${worldsens_sum}" = "wsnet1le"])
AM_CONDITIONAL([BUILD_WSNET2],     [test "${worldsens_sum}" = "wsnet2"])

if test "$wsnet1_sum" = yes; then
AC_DEFINE(WSNET1, [1], [Worldsens WSNet1 protocol])
fi

dnl if test "$wsnet1le_sum" = yes; then
dnl AC_DEFINE(WSNET1LE, [1], [Worldsens WSNet1LE protocol])
dnl fi

if test "$wsnet2_sum" = yes; then
AC_DEFINE(WSNET2, [1], [Worldsens WSNet2 protocol])
fi

dnl --------------------------------------------------------------
dnl Platform models
dnl --------------------------------------------------------------

NPLATFORM=0
PLATFORMNAMES=""

dnl wsn430 
AC_ARG_ENABLE([platform-wsn430],AS_HELP_STRING([--enable-platform-wsn430],[wsn430 platform]))
if test "${enable_platform_wsn430}" = "yes" ; then
	enable_mcu_msp430=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="wsn430"
fi

dnl senslab
AC_ARG_ENABLE([platform-senslab],AS_HELP_STRING([--enable-platform-senslab],[senslab platforms]))
if test "${enable_platform_senslab}" = "yes" ; then
	enable_mcu_msp430=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="senslab"
fi

dnl mosar
AC_ARG_ENABLE([platform-mosar],AS_HELP_STRING([--enable-platform-mosar],[mosar platforms]))
if test "${enable_platform_mosar}" = "yes" ; then
	enable_mcu_msp430=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="mosar"
fi

dnl OT setre 1
AC_ARG_ENABLE([platform-ot2006],AS_HELP_STRING([--enable-platform-ot2006],[OT 2006 platform]))
if test "${enable_platform_ot2006}" = "yes" ; then
	enable_mcu_msp430=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="ot2006 $PLATFORMNAMES"
fi

dnl OT setre 2007
AC_ARG_ENABLE([platform-ot2007],AS_HELP_STRING([--enable-platform-ot2007],[OT 2007 platform]))
if test "${enable_platform_ot2007}" = "yes" ; then
	enable_mcu_msp430=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="ot2007 $PLATFORMNAMES"
fi

dnl ez430
AC_ARG_ENABLE([platform-ez430],AS_HELP_STRING([--enable-platform-ez430],[TI-ez430 platform]))
if test "${enable_platform_ez430}" = "yes" ; then
	enable_mcu_msp430=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="ez430 $PLATFORMNAMES"
fi

dnl Test 
AC_ARG_ENABLE([platform-tests],AS_HELP_STRING([--enable-platform-tests],[test platform]))
if test "${enable_platform_tests}" = "yes" ; then
	enable_mcu_msp430=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="tests $PLATFORMNAMES"
fi

dnl Telosb
AC_ARG_ENABLE([platform-telosb],AS_HELP_STRING([--enable-platform-telosb],[telosb platform]))
if test "${enable_platform_telosb}" = "yes" ; then
	enable_mcu_msp430=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="telosb $PLATFORMNAMES"
fi

dnl CSEM WiseNodes
AC_ARG_ENABLE([platform-wisenode],AS_HELP_STRING([--enable-platform-wisenode],[wisenode platform]))
if test "${enable_platform_wisenode}" = "yes" ; then
	enable_mcu_msp430=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="wisenode $PLATFORMNAMES"
fi

dnl ICL BSN
AC_ARG_ENABLE([platform-iclbsn],AS_HELP_STRING([--enable-platform-iclbsn],[ICL BSN platform]))
if test "${enable_platform_iclbsn}" = "yes" ; then
	enable_mcu_msp430=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="iclbsn $PLATFORMNAMES"
fi

dnl Mica2
AC_ARG_ENABLE([platform-mica2],AS_HELP_STRING([--enable-platform-mica2],[mica2 platform]))
if test "${enable_platform_mica2}" = "yes" ; then
	enable_mcu_atmega=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="mica2 $PLATFORMNAMES"
fi

dnl MicaZ
AC_ARG_ENABLE([platform-micaz],AS_HELP_STRING([--enable-platform-micaz],[micaz platform]))
if test "${enable_platform_micaz}" = "yes" ; then
	enable_mcu_atmega=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="micaz $PLATFORMNAMES"
fi


dnl Default is to build all platforms
ALL="no"
if test $NPLATFORM -eq 0 ; then
  ALL="yes"
  PLATFORMNAMES="all"
  NPLATFORM=1 
fi

AM_CONDITIONAL([BUILD_WSN430],    [test "${enable_platform_wsn430}"    = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_SENSLAB],   [test "${enable_platform_senslab}"   = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_MOSAR],     [test "${enable_platform_mosar}"     = "yes"])
AM_CONDITIONAL([BUILD_OT2006],    [test "${enable_platform_ot2006}"    = "yes"])
AM_CONDITIONAL([BUILD_OT2007],    [test "${enable_platform_ot2007}"    = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_EZ430],     [test "${enable_platform_ez430}"     = "yes"])
AM_CONDITIONAL([BUILD_TESTS],     [test "${enable_platform_tests}"     = "yes"])
AM_CONDITIONAL([BUILD_TELOSB],    [test "${enable_platform_telosb}"    = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_WISENODE],  [test "${enable_platform_wisenode}"  = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_ICLBSN],    [test "${enable_platform_iclbsn}"    = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_MICA2],     [test "${enable_platform_mica2}"     = "yes"])
AM_CONDITIONAL([BUILD_MICAZ],     [test "${enable_platform_micaz}"     = "yes"])

AM_CONDITIONAL([BUILD_MSP430],    [test "${enable_mcu_msp430}"         = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_ATMEGA],    [test "${enable_mcu_atmega}"         = "yes" -o "$ALL" = "yes" ])

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
dnl AC_CHECK_SIZEOF(char, 1)
dnl AC_CHECK_SIZEOF(short, 2)
dnl AC_CHECK_SIZEOF(int *, 4)
dnl AC_CHECK_SIZEOF(int, 4)
dnl AC_CHECK_SIZEOF(long, 4)
dnl AC_CHECK_SIZEOF(long long, 8)

dnl --------------------------------------------------------------
dnl Checks for library functions.
dnl --------------------------------------------------------------
AC_FUNC_MEMCMP

dnl sys/resources.h
AC_CHECK_FUNC(getrusage)
if test "$ac_cv_func_getrusage" = yes; then
   AC_DEFINE(FUNC_GETRUSAGE_DEFINED, [1], [Check if getrusage() is available])
fi

dnl string.h
AC_CHECK_FUNC(bzero)
if test "$ac_cv_func_bzero" = yes; then
   AC_DEFINE(FUNC_BZERO_DEFINED, [1], [Check if bzero(3) is available])
else
   AC_DEFINE([bzero(s,n)],[memset(s,0,n)], [Macro replacement])
fi

AC_CHECK_FUNC(memset)
if test "$ac_cv_func_memset" = yes; then
   AC_DEFINE(FUNC_MEMSET_DEFINED, [1], [Check if memset(3) is available])
fi

AC_CHECK_FUNC(strsignal)
if test "$ac_cv_func_strsignal" = yes; then
   AC_DEFINE(FUNC_STRSIGNAL_DEFINED, [1], [Check if strsignal() is available])
fi

AC_CHECK_FUNC(strcasestr)
if test "$ac_cv_func_strcasestr" = yes; then
   AC_DEFINE(FUNC_STRCASESTR_DEFINED, [1], [Check if strcasestr() is available])
fi


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
dnl Checks for binutils (bfd) for msp430
dnl --------------------------------------------------------------
dnl should change to remove bfd option

bfd_sum="no"
AC_ARG_WITH([bfd],
	    AS_HELP_STRING([--with-bfd],[msp430 GNU binutils]),
	    [bfd_sum="yes"],
	    [])

AC_ARG_WITH([bfd-prefix],
	    AS_HELP_STRING([--with-bfd-prefix=DIR],[msp430 GNU binutils path]),
	    [CFLAGS="-I$withval/include $CFLAGS" LDFLAGS="-L$withval/lib -lz $LDFLAGS" bfd_sum="yes" bfd_prefix=$withval],
	    [bfd_prefix=""])

if test "${bfd_sum}" = "yes" ; then 
	AC_DEFINE([WSIM_USES_GNU_BFD],[1],[GNU BFD library (GPL)])
	EXTRALIBS="-lbfd -liberty $EXTRALIBS"
	if test "${bfd_prefix}" = "" ; then 
	     AC_MSG_NOTICE([== ---------------------------------------------------------------- ==])
             AC_MSG_NOTICE([== System configured with default libbfd,                           ==])
             AC_MSG_NOTICE([== please make sure that your installation supports MSP430 binaries ==])
             AC_MSG_NOTICE([== ---------------------------------------------------------------- ==])
        fi
fi
AM_CONDITIONAL([BUILD_GNUBFD],[test "${bfd_sum}" = "yes"])

elf_sum=""
if test "${bfd_sum}" = "yes" ; then 
	elf_sum="GNU BFD"
else
	elf_sum="internal"
fi

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
	;;

	x86_64-*-linux-* ) 
	# x86_64-unknown-linux-gnu
	HOSTTYPE="Linux"
	CFLAGS="$CFLAGS -DLINUX"
	NEEDLARGEFILES="yes"
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
dnl 

echo "EXTRALIBS ${EXTRALIBS}"
AC_SUBST(EXTRALIBS)

AC_CONFIG_FILES(
dnl 
arch/Makefile
arch/common/Makefile
arch/msp430/Makefile
arch/atmega/Makefile
dnl
devices/Makefile
devices/led/Makefile
devices/7seg/Makefile
devices/m25p80/Makefile
devices/at45db/Makefile
devices/ds2411/Makefile
devices/gdm1602a/Makefile
devices/hd44780/Makefile
devices/bargraph/Makefile
devices/ptty/Makefile
devices/cc1100/Makefile
devices/cc2420/Makefile
devices/uigfx/Makefile
dnl
platforms/Makefile
platforms/wsn430/Makefile
platforms/senslab/Makefile
platforms/mosar/Makefile
platforms/ez430/Makefile
platforms/ot2006/Makefile
platforms/ot2007/Makefile
platforms/tests/Makefile
platforms/telosb/Makefile
platforms/wisenode/Makefile
platforms/mica2/Makefile
platforms/micaz/Makefile
platforms/iclbsn/Makefile
dnl
libelf/Makefile
libgdb/Makefile
libgui/Makefile
liblogger/Makefile
libetrace/Makefile
libtracer/Makefile
libselect/Makefile
libconsole/Makefile
libwsnet/Makefile
machine/Makefile
src/Makefile
dnl
doc/Makefile
dnl
Makefile
)
AC_OUTPUT

dnl --------------------------------------------------------------
dnl Summary
dnl --------------------------------------------------------------
if test ${NPLATFORM} -eq 0 ; then
	AC_MSG_NOTICE([== --------------------------------------------------- ==])
	AC_MSG_NOTICE([== You need to select one simulation platform          ==])
	AC_MSG_NOTICE([== Please re-configure before compiling                ==])
	AC_MSG_NOTICE([== --------------------------------------------------- ==])
fi

echo ""
echo "Configuration summary:"
echo ""
echo "        host         : ${host}"
echo "        host type    : $HOSTTYPE"
echo "        target(s)    : $PLATFORMNAMES"
echo "        wsim debug   : ${debug_sum}"
echo "        target debug : ${xdebug_sum}"
echo "        Etrace       : ${etrace_sum}"
echo "        Elf loader   : ${elf_sum}"
echo "        SDL gui      : ${sdl_sum}"
echo "        zlib         : ${zlib_sum}"
echo "        Ptty         : ${ptty_sum}"
echo "        WorldSens    : ${worldsens_sum}"
echo ""
