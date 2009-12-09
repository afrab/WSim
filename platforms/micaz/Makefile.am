if BUILD_MICAZ

bin_PROGRAMS=wsim-micaz

if BUILD_PTTY
EXTRADEV= ../../devices/ptty/libptty.a
endif

INCLUDES=-I$(top_srcdir)

MICAZMCU= ../../arch/atmega/libatmega128l.a
MICAZDEV= ../../devices/led/libled.a			\
		../../devices/ds2411/libds2411.a	\
		../../devices/m25p80/libm25p80.a	\
		${EXTRADEV}

wsim_micaz_CFLAGS=-DATMEGA128L
wsim_micaz_SOURCES=micaz.c
wsim_micaz_LDADD=${MICAZMCU} ${MICAZDEV} ${WSIMADD}  	\
	../../machine/libmachine.a ${MICAZMCU} ../../devices/libdevices.a ../../libelf/libelf.a
wsim_micaz_DEPENDENCIES=${wsim_micaz_LDADD}

## WMICAZDEV=../../devices/cc2420/libcc2420.a 	\
##	../../devices/worldsens/libworldsens.a

endif
