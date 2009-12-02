if BUILD_MOSAR

bin_PROGRAMS=wsim-mosar

INCLUDES=-I$(top_srcdir)

MOSAR_MCU= ../../arch/msp430/libmsp430f2274.a
MOSAR_DEV= ../../devices/led/libled.a			\
		../../devices/m25p/libm25p64.a		\
		../../devices/ptty/libptty.a		\
		../../devices/cc1100_2500/libcc1100.a   \
		../../devices/uigfx/libuigfx.a

## v1: mosar model
wsim_mosar_CFLAGS=-DMSP430f2274 -DCC1100
wsim_mosar_SOURCES=mosar.c
wsim_mosar_LDADD=${MOSAR_DEV} ${WSIMADD} ${MOSAR_MCU}

endif
