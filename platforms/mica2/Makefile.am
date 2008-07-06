if BUILD_MICA2

bin_PROGRAMS=wsim-mica2
## if BUILD_WORLDSENS
## bin_PROGRAMS+=worldsens-mica2
## endif

if BUILD_PTTY
EXTRADEV= ../../devices/ptty/libptty.a
endif

INCLUDES=-I$(top_srcdir)

MICA2MCU= ../../arch/atmega/libatmega128l.a
MICA2DEV= ../../devices/led/libled.a			\
		../../devices/ds2411/libds2411.a	\
		../../devices/m25p80/libm25p80.a	\
		${EXTRADEV}

wsim_mica2_CFLAGS=-DATMEGA128L
wsim_mica2_SOURCES=mica2.c
wsim_mica2_LDADD=${MICA2MCU} ${MICA2DEV} ${WSIMADD}  ../../machine/libmachine.a ${MICA2MCU} ../../devices/libdevices.a ../../libelf/libelf.a



## WMICA2DEV=../../devices/cc1000/libcc1000.a 	\
##	../../devices/worldsens/libworldsens.a

## worldsens_mica2_CFLAGS=-DATMEGA128L -DWORLDSENS
## worldsens_mica2_SOURCES=mica2.c
## worldsens_mica2_LDADD=${MICA2MCU} ${MICA2DEV} ${WMICA2DEV} ${WSIMADD}

endif
