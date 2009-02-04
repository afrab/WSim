if BUILD_OTSETRE

bin_PROGRAMS=wsim-otsetre

INCLUDES=-I$(top_srcdir)

OTSETREMCU= ../../arch/msp430/libmsp430f149.a
OTSETREDEV= ../../devices/led/libled.a      	\
	../../devices/7seg/lib7seg.a            \
	../../devices/hd44780/libhd44780.a      \
	../../devices/bargraph/libbargraph.a    \
	../../devices/uigfx/libuigfx.a          \
	../../devices/ptty/libptty.a

wsim_otsetre_CFLAGS=-DMSP430f149
wsim_otsetre_SOURCES=otsetre.c
wsim_otsetre_LDADD=${OTSETREDEV} ${WSIMADD} ${OTSETREMCU}

endif
