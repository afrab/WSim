if BUILD_EZ430CHRONOS

bin_PROGRAMS=wsim-ez430chronos

INCLUDES=-I$(top_srcdir)

EZ430CHRONOS_MCU= ../../arch/msp430/libcc430f6137.a
EZ430CHRONOS_DEV= ../../devices/led/libled.a \
		  ../../devices/7seg/lib7seg.a \
		  ../../devices/cma3000_spi/libcma3000_spi.a \
		  ../../devices/scp1000_i2c/libscp1000_i2c.a \
		  ../../devices/ez430_lcd/libez430_lcd.a


## v1: model
wsim_ez430chronos_CFLAGS=-DCC430f6137 -DCC1101MM -DCC1100
wsim_ez430chronos_SOURCES=ez430chronos.c
wsim_ez430chronos_LDADD=${EZ430CHRONOS_DEV} ${WSIMADD} ${EZ430CHRONOS_MCU} ../../devices/cc1100_2500/libcc1101mm.a
wsim_ez430chronos_DEPENDENCIES=${wsim_ez430chronos_LDADD}
endif
