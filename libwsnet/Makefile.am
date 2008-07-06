

noinst_LIBRARIES=libwsnet0.a

if BUILD_WORLDSENS
if BUILD_WSNET1
noinst_LIBRARIES+=libwsnet1.a
endif

## if BUILD_WSNET1LE
## noinst_LIBRARIES+=libwsnet1le.a
## endif

if BUILD_WSNET2
noinst_LIBRARIES+=libwsnet2.a
endif
endif

INCLUDES=-I$(top_srcdir)



libwsnet0_a_CFLAGS=
libwsnet0_a_SOURCES=libwsnet.h	\
		wsnet0.h 	\
		wsnet0.c

libwsnet1_a_CFLAGS=
libwsnet1_a_SOURCES=libwsnet.h 	\
		wsnet1.h 	\
		wsnet1.c	\
		wsnet1_dbg.h 	\
		pktlist.h	\
		pktlist.c

## libwsnet1le_a_CFLAGS=-I${srcdir}/../utils/wsnet1le/src
## libwsnet1le_a_SOURCES=libwsnet.h \
##		wsnet1le.h 	\
##		wsnet1le.c	\
##		wsnet1_dbg.h 	\
##		pktlist.h	\
##		pktlist.c

libwsnet2_a_CFLAGS=
libwsnet2_a_SOURCES=libwsnet.h	\
		wsnet2.h	\
		wsnet2.c	\
		wsnet2_net.c	\
		wsnet2_pkt.h 	\
		wsnet2_pkt.c
