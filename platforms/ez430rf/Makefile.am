if BUILD_EZ430RF

bin_PROGRAMS=wsim-ez430rf
if BUILD_WORLDSENS
bin_PROGRAMS+=worldsens-ez430rf
endif

INCLUDES=-I$(top_srcdir)

EZ430RF_MCU= ../../arch/msp430/libmsp430f2274.a
EZ430RF_DEV= ../../devices/led/libled.a			\
		../../devices/ptty/libptty.a		\
		../../devices/cc1100/libcc1100.a        \
		../../devices/uigfx/libuigfx.a

## v1: model
wsim_ez430rf_CFLAGS=-DMSP430f2274
wsim_ez430rf_SOURCES=ez430rf.c
wsim_ez430rf_LDADD=${EZ430RF_DEV} ${WSIMADD} ${EZ430RF_MCU}

worldsens_ez430rf_CFLAGS=-DMSP430f2274
worldsens_ez430rf_SOURCES=ez430rf.c
worldsens_ez430rf_LDADD=${EZ430RF_DEV} ${WSIMADD_WORLDSENS} ${EZ430RF_MCU}

endif
