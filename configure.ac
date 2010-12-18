dnl -*-m4-*-
dnl Process this file with autoconf to produce a configure script.
dnl --------------------------------------------------------------
dnl configure.ac for Worldsens architecture simulator
dnl works only for automake 1.10 and above 
dnl lower versions will have problem with bzero prototype replacement
dnl --------------------------------------------------------------
AC_INIT([wsim],[0.99.405svn])
AC_CONFIG_SRCDIR(src/main.c)
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE(foreign) dnl foreign = NEWS/TODO not required
AC_CONFIG_HEADERS([config.h])
 
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

EXTRAOBJS=""
EXTRALIBS="-lm -lz"

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

dnl Indent option : enable code indentation
dnl ============ 
# The indent target require GNU indent
AC_PATH_PROG([INDENT], [indent])
AC_PATH_PROG([GINDENT], [gindent])
if test x"$GINDENT" != x""; then
  INDENT="$GINDENT"
fi

AC_SUBST(INDENT)
AM_CONDITIONAL(HAS_INDENT, [test x"$INDENT" != x""])

dnl Debug mode : enable debug code for wsim internals
dnl ============ 
CFLAGS_save="${CFLAGS}"
AC_ARG_ENABLE([debug],AS_HELP_STRING([--enable-debug],[wsim debug mode (default enabled)]))
if test "${enable_debug}" != "no"; then
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
    AC_DEFINE([XCODE_DEBUG],       [1],[WSim target code debug tools  ])
    AC_DEFINE([ENABLE_RAM_CONTROL],[1],[WSim target ram access control])
else
    xdebug_sum="no"
fi
AM_CONDITIONAL([BUILD_XCODE_DEBUG], [test "${xdebug_sum}" != "no"])

dnl eTrace mode
dnl ===========
CFLAGS_save="${CFLAGS}"
AC_ARG_ENABLE([etrace],AS_HELP_STRING([--enable-etrace],[etrace mode (default disabled)]))
if test "${enable_etrace}" = "yes"; then
    AC_CHECK_LIB(z, compress)
    AC_DEFINE([ETRACE],[1],[eSimu energy tracer])
    etrace_sum="yes"
else
    etrace_sum="no"
fi
AM_CONDITIONAL([BUILD_ETRACE], [test "${etrace_sum}" = "yes"])

AC_ARG_WITH([etracelib-prefix], AS_HELP_STRING([--with-etracelib-prefix=DIR],[etracelib path]),
	    [CFLAGS="-I$withval $CFLAGS" 
	     LDFLAGS="-L$withval $LDFLAGS"
	     EXTRAOBJS="$withval/libetrace.a $EXTRAOBJS"

	     etracelib_prefix=$withval],
	    [etracelib_prefix=""])

zlib_sum="system" 
AC_ARG_WITH([zlib-prefix], AS_HELP_STRING([--with-zlib-prefix=DIR],[zlib path]),
	    [CFLAGS="-I$withval/include $CFLAGS" 
	     LDFLAGS="-L$withval/lib $LDFLAGS" 
	     zlib_sum="yes" 
	     zlib_prefix=$withval],
	    [zlib_prefix=""])

dnl UI mode
dnl ===========
gui_type="none"
AC_ARG_ENABLE([gui],AS_HELP_STRING([--enable-gui],[GUI (default enabled)]))
if test "${enable_gui}" != "no" ; then
    gui_sum="yes"
    AC_DEFINE([GUI],[1],[Defines wether a GUI is available])
else
    gui_sum="no"
fi
AM_CONDITIONAL([BUILD_GUI], [test "${gui_sum}" = "yes"])

dnl dnl SDL ui mode
dnl dnl ===========
dnl AC_ARG_ENABLE([sdl],AS_HELP_STRING([--enable-sdl],[SDL GUI for devices (default disabled)]))
dnl if test "${enable_sdl}" = "yes" ; then
dnl     gui_type="SDL"
dnl     AC_DEFINE([GUI_SDL],[1],[Defines wether a SDL GUI is available])
dnl fi
AM_CONDITIONAL([BUILD_GUI_SDL], [test "${gui_type}" = "SDL_is_deprecated"])

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
AC_ARG_ENABLE([ptty],AS_HELP_STRING([--enable-ptty],[Pseudo TTY IO device (default enabled)]))
if test "${enable_ptty}" != "no" ; then
   ptty_sum="yes"
   AC_DEFINE([PTTY],[1],[Defines wether libptty is used on the corrent configuration])
else
   ptty_sum="no"
fi
AM_CONDITIONAL([BUILD_PTTY], [test "${ptty_sum}" = "yes"])

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

dnl OT setre 1
AC_ARG_ENABLE([platform-ot2006],AS_HELP_STRING([--enable-platform-ot2006],[OT 2006 platform]))
if test "${enable_platform_ot2006}" = "yes" ; then
	enable_mcu_msp430=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="ot2006 $PLATFORMNAMES"
fi

dnl OT setre 2007
AC_ARG_ENABLE([platform-otsetre],AS_HELP_STRING([--enable-platform-otsetre],[OT 2007 platform]))
if test "${enable_platform_otsetre}" = "yes" ; then
	enable_mcu_msp430=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="otsetre $PLATFORMNAMES"
fi

dnl ez430
AC_ARG_ENABLE([platform-ez430],AS_HELP_STRING([--enable-platform-ez430],[TI-ez430 platform]))
if test "${enable_platform_ez430}" = "yes" ; then
	enable_mcu_msp430=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="ez430 $PLATFORMNAMES"
fi

dnl ez430-rf2500
AC_ARG_ENABLE([platform-ez430rf],AS_HELP_STRING([--enable-platform-ez430rf],[ez430rf2500 platforms]))
if test "${enable_platform_ez430rf}" = "yes" ; then
	enable_mcu_msp430=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="ez430rf"
fi

dnl mosar
AC_ARG_ENABLE([platform-mosar],AS_HELP_STRING([--enable-platform-mosar],[mosar platforms]))
if test "${enable_platform_mosar}" = "yes" ; then
	enable_mcu_msp430=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="mosar"
fi

dnl mcugen
AC_ARG_ENABLE([platform-mcugen],AS_HELP_STRING([--enable-platform-mcugen],[mcugen platform]))
if test "${enable_platform_mcugen}" = "yes" ; then
	enable_mcu_mcugen=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="mcugen $PLATFORMNAMES"
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

dnl WASP project
AC_ARG_ENABLE([platform-wasp],AS_HELP_STRING([--enable-platform-wasp],[wasp platforms]))
if test "${enable_platform_wasp}" = "yes" ; then
	enable_mcu_msp430=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="wasp $PLATFORMNAMES"
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

dnl Powwow
AC_ARG_ENABLE([platform-powwow],AS_HELP_STRING([--enable-platform-powwow],[Powwow platform]))
if test "${enable_platform_powwow}" = "yes" ; then
	enable_mcu_msp430=yes
	NPLATFORM=$(($NPLATFORM + 1))
	PLATFORMNAMES="micaz $PLATFORMNAMES"
fi

dnl All
ALL="no"
AC_ARG_ENABLE([platform-all],AS_HELP_STRING([--enable-platform-all],[all platforms]))
if test "${enable_platform_all}" = "yes" ; then
        ALL="yes"
	NPLATFORM=1
	PLATFORMNAMES="all $PLATFORMNAMES"
fi

dnl Default is to build all platforms
if test $NPLATFORM -eq 0 ; then
  ALL="yes"
  PLATFORMNAMES="all"
  NPLATFORM=1 
fi

AM_CONDITIONAL([BUILD_WSN430],    [test "${enable_platform_wsn430}"      = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_SENSLAB],   [test "${enable_platform_senslab}"     = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_OT2006],    [test "${enable_platform_ot2006}"      = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_OTSETRE],   [test "${enable_platform_otsetre}"     = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_EZ430],     [test "${enable_platform_ez430}"       = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_TESTS],     [test "${enable_platform_tests}"       = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_TELOSB],    [test "${enable_platform_telosb}"      = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_WASP],      [test "${enable_platform_wasp}"        = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_WISENODE],  [test "${enable_platform_wisenode}"    = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_ICLBSN],    [test "${enable_platform_iclbsn}"      = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_EZ430RF],   [test "${enable_platform_ez430rf}"     = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_MCUGENP],   [test "${enable_platform_mcugenp}"     = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_MOSAR],     [test "${enable_platform_mosar}"       = "yes"])
AM_CONDITIONAL([BUILD_MICA2],     [test "${enable_platform_mica2}"       = "yes" -o "$ALL" = "yes"])
AM_CONDITIONAL([BUILD_MICAZ],     [test "${enable_platform_micaz}"       = "yes" -o "$ALL" = "yes"])
AM_CONDITIONAL([BUILD_POWWOW],    [test "${enable_platform_powwow}"      = "yes" -o "$ALL" = "yes"])

AM_CONDITIONAL([BUILD_MSP430],    [test "${enable_mcu_msp430}"         = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_ATMEGA],    [test "${enable_mcu_atmega}"         = "yes" -o "$ALL" = "yes" ])
AM_CONDITIONAL([BUILD_MCUGEN],    [test "${enable_mcu_mcugen}"         = "yes" -o "$ALL" = "yes" ])

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

AC_CHECK_FUNC(strtok_r)
if test "$ac_cv_func_strtok_r" = yes; then
   AC_DEFINE(FUNC_STRTOK_R_DEFINED, [1], [Check if strtok_r() is available])
fi

dnl AC_CHECK_FUNC(nothl)
dnl if test "$ac_cv_func_ntohl" = yes; then
dnl    AC_DEFINE(FUNC_NTOHL_DEFINED, [1], [Check if ntohl() is available])
dnl fi


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
	    [CFLAGS="-I$withval/include $CFLAGS" LDFLAGS="-L$withval/lib $LDFLAGS" bfd_sum="yes" bfd_prefix=$withval],
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

	*-*-linux* ) 
	# linux
	# i686-*-linux-* ) 
	# i686-pc-linux-gnu )
	# x86_64-*-linux-* ) 
	# x86_64-unknown-linux-gnu
	# i686-suse-linux )
	HOSTTYPE="Linux"
	CFLAGS="$CFLAGS -DLINUX"
	NEEDLARGEFILES="yes"
	EXTRALIBS="$EXTRALIBS"
	# backup GUI for unix machines
	if test "${gui_sum}" = "yes" -a "${gui_type}" = "none" ; then 
	    gui_type="X11"
	fi
	;;
   
	*-*-solaris* )
        # cimirly  sparc-sun-solaris2.9
        # clash    sparc-sun-solaris2.10
	HOSTTYPE="Solaris"
	CFLAGS="$CFLAGS -DSOLARIS"
	EXTRALIBS="$EXTRALIBS -lsocket -lnsl"
	# backup GUI for unix machines
	if test "${gui_sum}" = "yes" -a "${gui_type}" = "none" ; then 
	    gui_type="X11"
	fi
	;;

	*-*-cygwin )
	# xp i686-pc-cygwin
	HOSTTYPE="Cygwin"
	NEEDLARGEFILES="yes"
	if test "${gui_sum}" = "yes" -a "${gui_type}" = "none" ; then 
	    gui_type="WIN"
	    # the contradictory setting of subsystems below is needed because
	    # we need a console application, yet we need the GUI libraries that
	    # -mwindows adds to the link command.  (We could probably also get
	    # away with -mconsole -lgdi32 -lcomdlg32 for now - this is with
	    # Cygwin 1.7's gcc 4.3; but let's rather have the compiler sort
	    # that out for us.)
	    EXTRALIBS="$EXTRALIBS -mwindows -mconsole"
	fi
	;;

	*-*-mingw32* )
	# i586-pc-mingw32msvc
	# i686-pc-mingw32
	HOSTTYPE="Mingw32"
	NEEDLARGEFILES="yes"
	EXTRALIBS="$EXTRALIBS -lws2_32 -mwindows -lmingw32"
	if test "${gui_sum}" = "yes" -a "${gui_type}" = "none" ; then 
	    gui_type="WIN"
	fi
dnl 	if test "${gui_sum}" = "yes" -a "${gui_type}" = "SDL" ; then
dnl 	   # hack to avoid missing SDL_main linker complaint
dnl 	   LDFLAGS="$LDFLAGS -u_SDL_main"
dnl 	fi
	;;

	*-apple-darwin* )
        # powerpc-apple-darwin8.6.0 
	# i686-apple-darwin8.9.1
	# i386-apple-darwin9.8.0
	HOSTTYPE="MacOSX"
	CFLAGS="$CFLAGS -DMACOSX"
	NEEDLARGEFILES="yes"
	if test "${gui_sum}" = "yes" -a "${gui_type}" = "none" ; then 
	   # use X11 version until someone builds a Cocoa interface
	   gui_type="X11"
	   LDFLAGS="$LDFLAGS -L/usr/X11/lib"
	fi
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

AM_CONDITIONAL([BUILD_GUI_X11], [test "${gui_type}" = "X11"])
if test "${gui_type}" = "X11" ; then
   AC_DEFINE([GUI_X11], [1], [Defines wether a X11 GUI is available])
   EXTRALIBS="$EXTRALIBS -lX11"
fi

AM_CONDITIONAL([BUILD_GUI_WIN], [test "${gui_type}" = "WIN"])
if test "${gui_type}" = "WIN" ; then
   AC_DEFINE([GUI_WIN], [1], [Defines wether a Windows native GUI is available])
   EXTRALIBS="$EXTRALIBS"
fi

AM_CONDITIONAL([BUILD_GUI_NONE], [test "${gui_type}" = "none"])
if test "${gui_type}" = "none" ; then
   AC_DEFINE([GUI_NONE], [1], [Defines wether No-GUI is selected])
fi

#if test "${zlib_sum}" = "system" ; then 
#   EXTRALIBS="$EXTRALIBS -lz"
#fi

dnl --------------------------------------------------------------
dnl Checks for libraries.
dnl --------------------------------------------------------------
dnl if test "${enable_sdl}" = "yes" ; then
dnl 	SDL_VERSION=1.2.5
dnl 	AM_PATH_SDL($SDL_VERSION,   :,  AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])	)
dnl 
dnl 	if test "$HOSTTYPE" = "Cygwin" ; then 
dnl 		AC_MSG_NOTICE([stripping MinGW cruft from SDL_CFLAGS/_LIBS for Worldsens on Cygwin])
dnl 		SDL_CFLAGS="$(echo "$SDL_CFLAGS" | sed -e 's,-I/usr/include/mingw,,;s,-mno-cygwin,,;s,-Dmain=SDL_main,,' )"
dnl 		SDL_LIBS="$(echo "$SDL_LIBS" | sed -e 's,-lmingw32,,;s,-mno-cygwin,,;s,-mwindows,,;s,-lSDLmain,,' )"
dnl 	fi
dnl 
dnl 	CFLAGS="$CFLAGS $SDL_CFLAGS"
dnl 	EXTRALIBS="$EXTRALIBS $SDL_LIBS"
dnl fi

dnl Wextra && Werror
dnl ================
CFLAGS_save="${CFLAGS}"
AC_ARG_ENABLE([waou],AS_HELP_STRING([--enable-waou],[Wextra Werror mode (default disabled)]))
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

LIBS=${EXTRALIBS}

AC_MSG_NOTICE(EXTRALIBS=${EXTRALIBS})
AC_MSG_NOTICE(EXTRAOBJS=${EXTRAOBJS})

AC_SUBST(EXTRALIBS)
AC_SUBST(EXTRAOBJS)

AC_CONFIG_FILES(
dnl 
arch/Makefile
arch/common/Makefile
arch/msp430/Makefile
arch/atmega/Makefile
arch/mcugen/Makefile
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
devices/cc1100_2500/Makefile
devices/cc2420/Makefile
devices/uigfx/Makefile
devices/spidev/Makefile
devices/spidev_master/Makefile
devices/spidev_dsp/Makefile
dnl
platforms/Makefile
platforms/wsn430/Makefile
platforms/senslab/Makefile
platforms/mosar/Makefile
platforms/ez430/Makefile
platforms/ez430rf/Makefile
platforms/ot2006/Makefile
platforms/otsetre/Makefile
platforms/tests/Makefile
platforms/telosb/Makefile
platforms/mica2/Makefile
platforms/micaz/Makefile
platforms/iclbsn/Makefile
platforms/mcugen/Makefile
platforms/wasp/Makefile
platforms/wisenode/Makefile
platforms/powwow/Makefile
dnl
libelf/Makefile
libgdb/Makefile
libgui/Makefile
liblogger/Makefile
liblogpkt/Makefile
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

AC_MSG_NOTICE([


Configuration summary:

        host         : ${host}
        host type    : $HOSTTYPE
        target(s)    : $PLATFORMNAMES
        wsim debug   : ${debug_sum}
        target debug : ${xdebug_sum}
        Etrace       : ${etrace_sum}
        Elf loader   : ${elf_sum}
	GUI          : ${gui_sum} ${gui_type}
        zlib         : ${zlib_sum}
        Ptty         : ${ptty_sum}
])
