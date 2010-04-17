if BUILD_WASP

bin_PROGRAMS=wsim-wasp wsim-waspx

INCLUDES=-I$(top_srcdir)

WASP_MCU= ../../arch/msp430/libmsp430f1611.a

WASP_DEV= ../../devices/led/libled.a		        \
	    ../../devices/at45db/libat45db041b.a	\
            ../../devices/ptty/libptty.a                \
            ../../devices/cc2420/libcc2420.a		\
	    ../../devices/uigfx/libuigfx.a

WASPX_DEV= ../../devices/led/libled.a		        \
	../../devices/ptty/libptty.a            	\
	../../devices/spidev_dsp/libspidsp.a		\
	../../devices/uigfx/libuigfx.a


wsim_wasp_CFLAGS=-DMSP430f1611 
wsim_wasp_SOURCES=wasp.c
wsim_wasp_LDADD=${WASP_DEV} ${WSIMADD} ${WASP_MCU}
wsim_wasp_DEPENDENCIES=${wsim_wasp_LDADD}

wsim_waspx_CFLAGS=-DMSP430f1611 
wsim_waspx_SOURCES=waspx.c
wsim_waspx_LDADD=${WASPX_DEV} ${WSIMADD} ${WASP_MCU}
wsim_waspx_DEPENDENCIES=${wsim_waspx_LDADD}

endif
