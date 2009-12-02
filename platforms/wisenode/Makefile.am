if BUILD_WISENODE

bin_PROGRAMS=wsim-wisenode

INCLUDES=-I$(top_srcdir)

WISENODE_MCU= ../../arch/msp430/libmsp430f1611.a
WISENODE_DEV= ../../devices/led/libled.a		\
		../../devices/m25p80/libm25p10.a	\
		../../devices/ptty/libptty.a		\
		../../devices/cc1100_2500/libcc1100.a	\
		../../devices/uigfx/libuigfx.a

wsim_wisenode_CFLAGS=-DMSP430f1611 -DCC1100
wsim_wisenode_SOURCES=wisenode.c
wsim_wisenode_LDADD=${WISENODE_DEV} ${WSIMADD} ${WISENODE_MCU}

endif
