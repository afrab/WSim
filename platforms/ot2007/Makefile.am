if BUILD_OT2007

bin_PROGRAMS=wsim-ot2007

INCLUDES=-I$(top_srcdir)

OT2007MCU= ../../arch/msp430/libmsp430f149.a
OT2007DEV= ../../devices/led/libled.a      	\
	../../devices/7seg/lib7seg.a            \
	../../devices/hd44780/libhd44780.a      \
	../../devices/bargraph/libbargraph.a    \
	../../devices/uigfx/libuigfx.a          \
	../../devices/ptty/libptty.a

wsim_ot2007_CFLAGS=-DMSP430f149
wsim_ot2007_SOURCES=ot2007.c
wsim_ot2007_LDADD=${OT2007DEV} ${WSIMADD} ${OT2007MCU}

endif
