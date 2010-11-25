if BUILD_TESTS

bin_PROGRAMS=wsim-powwow

INCLUDES=-I$(top_srcdir)

MCU=../../arch/msp430/libmsp430f1611.a

TEST_DEV=../../devices/led/libled.a   		\
	../../devices/ptty/libptty.a		\
	../../devices/spidev/libspidev.a	\
	../../devices/uigfx/libuigfx.a

wsim_powwow_CFLAGS=-DMSP430f1611
wsim_powwow_SOURCES=powwow.c
wsim_powwow_LDADD=${TEST_DEV} ${WSIMADD} ${MCU} 
wsim_powwow_DEPENDENCIES=${wsim_powwow_LDADD}

endif
