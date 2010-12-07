if BUILD_POWWOW

bin_PROGRAMS=wsim-powwow

INCLUDES=-I$(top_srcdir)

MCU=../../arch/msp430/libmsp430f1612.a

TEST_DEV=../../devices/led/libled.a   		\
	../../devices/ptty/libptty.a		\
	../../devices/spidev/libspidev.a	\
	../../devices/cc2420/libcc2420.a	\
	../../devices/uigfx/libuigfx.a

wsim_powwow_CFLAGS=-DMSP430f1612
wsim_powwow_SOURCES=powwow.c
wsim_powwow_LDADD=${TEST_DEV} ${WSIMADD} ${MCU} 
wsim_powwow_DEPENDENCIES=${wsim_powwow_LDADD}

endif
