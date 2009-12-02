if BUILD_EZ430RF

bin_PROGRAMS=wsim-ez430rf

INCLUDES=-I$(top_srcdir)

EZ430RF_MCU= ../../arch/msp430/libmsp430f2274.a
EZ430RF_DEV= ../../devices/led/libled.a			\
		../../devices/ptty/libptty.a		\
		../../devices/cc1100_2500/libcc2500.a   \
		../../devices/uigfx/libuigfx.a

## v1: model
wsim_ez430rf_CFLAGS=-DMSP430f2274 -DCC2500
wsim_ez430rf_SOURCES=ez430rf.c
wsim_ez430rf_LDADD=${EZ430RF_DEV} ${WSIMADD} ${EZ430RF_MCU}

endif
