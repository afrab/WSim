if BUILD_EZ430

bin_PROGRAMS=wsim-ez430

INCLUDES=-I$(top_srcdir)

EZ430MCU=../../arch/msp430/libmsp430f2013.a

EZ430DEV= ../../devices/led/libled.a		\
		../../devices/uigfx/libuigfx.a

wsim_ez430_CFLAGS=-DMSP430f2013
wsim_ez430_SOURCES=ez430.c
wsim_ez430_LDADD=${EZ430DEV} ${WSIMADD} ${EZ430MCU}
wsim_ez430_DEPENDENCIES=${wsim_ez430_LDADD}
endif
