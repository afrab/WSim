if BUILD_TESTS

bin_PROGRAMS=wsim-msp135 \
		wsim-msp1611

INCLUDES=-I$(top_srcdir)

MCU_135=../../arch/msp430/libmsp430f135.a
MCU_1611=../../arch/msp430/libmsp430f1611.a

if BUILD_PTTY
EXTRADEV= ../../devices/ptty/libptty.a
endif

TEST_DEV=../../devices/led/libled.a   \
	${EXTRADEV}


wsim_msp135_CFLAGS=-DMSP430f135
wsim_msp135_SOURCES=test1.c
wsim_msp135_LDADD=${TEST_DEV} ${WSIMADD} ${MCU_135} 

wsim_msp1611_CFLAGS=-DMSP430f1611
wsim_msp1611_SOURCES=test1.c
wsim_msp1611_LDADD=${TEST_DEV} ${WSIMADD} ${MCU_1611} 

endif
