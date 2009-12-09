if BUILD_TELOSB

bin_PROGRAMS=wsim-telosb

INCLUDES=-I$(top_srcdir)

TELOSB_MCU= ../../arch/msp430/libmsp430f1611.a
TELOSB_DEV= ../../devices/led/libled.a			\
		../../devices/ds2411/libds2411.a	\
		../../devices/m25p80/libm25p80.a	\
		../../devices/ptty/libptty.a		\
		../../devices/cc2420/libcc2420.a


wsim_telosb_CFLAGS=-DMSP430f1611
wsim_telosb_SOURCES=telosb.c
wsim_telosb_LDADD=${TELOSB_DEV} ${WSIMADD} ${TELOSB_MCU} 
wsim_telosb_DEPENDENCIES=${wsim_telosb_LDADD}

endif
