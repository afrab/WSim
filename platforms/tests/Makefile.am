if BUILD_TESTS

bin_PROGRAMS=wsim-msp135 wsim-msp1611 wsim-msp1611-2

INCLUDES=-I$(top_srcdir)

MCU_135=../../arch/msp430/libmsp430f135.a
MCU_1611=../../arch/msp430/libmsp430f1611.a

TEST_DEV=../../devices/led/libled.a   		\
	../../devices/ptty/libptty.a		\
	../../devices/spidev/libspidev.a	\
	../../devices/uigfx/libuigfx.a

wsim_msp135_CFLAGS=-DMSP430f135
wsim_msp135_SOURCES=test1.c
wsim_msp135_LDADD=${TEST_DEV} ${WSIMADD} ${MCU_135} 
wsim_msp135_DEPENDENCIES=${wsim_msp135_LDADD}

wsim_msp1611_CFLAGS=-DMSP430f1611
wsim_msp1611_SOURCES=test1.c
wsim_msp1611_LDADD=${TEST_DEV} ${WSIMADD} ${MCU_1611} 
wsim_msp1611_DEPENDENCIES=${wsim_msp1611_LDADD}

wsim_msp1611_2_CFLAGS=-DMSP430f1611
wsim_msp1611_2_SOURCES=test2.c
wsim_msp1611_2_LDADD=${TEST_DEV} ${WSIMADD} ${MCU_1611} 
wsim_msp1611_2_DEPENDENCIES=${wsim_msp1611_2_LDADD}

endif
