if BUILD_OT2006

bin_PROGRAMS=wsim-ot2006

INCLUDES=-I$(top_srcdir)

OT2006MCU= ../../arch/msp430/libmsp430f449.a
OT2006DEV= ../../devices/led/libled.a		\
	../../devices/7seg/lib7seg.a		\
	../../devices/gdm1602a/libgdm1602a.a	\
        ../../devices/ptty/libptty.a

wsim_ot2006_CFLAGS=-DMSP430f449
wsim_ot2006_SOURCES=ot2006.c
wsim_ot2006_LDADD=${OT2006DEV} ${WSIMADD} ${OT2006MCU}

endif
