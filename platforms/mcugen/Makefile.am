if BUILD_MCUGENP

bin_PROGRAMS=wsim-mcugen

INCLUDES=-I$(top_srcdir)

MCU=../../arch/mcugen/libmcugen.a

DEV=../../devices/led/libled.a   		\
	../../devices/uigfx/libuigfx.a

wsim_mcugen_CFLAGS=-DMCUGEN
wsim_mcugen_SOURCES=mcugenp.c
wsim_mcugen_LDADD=${DEV} ${WSIMADD} ${MCU} 
wsim_mcugen_DEPENDENCIES=${wsim_mcugen_LDADD}

endif
