if BUILD_SENSLAB

bin_PROGRAMS=wsim-senslabv13b wsim-senslabv14

INCLUDES=-I$(top_srcdir)

SENSLAB_MCU= ../../arch/msp430/libmsp430f1611.a
SENSLAB_DEV= ../../devices/led/libled.a			\
		../../devices/ds2411/libds2411.a	\
		../../devices/m25p80/libm25p80.a	\
		../../devices/ptty/libptty.a		\
		../../devices/uigfx/libuigfx.a

V13B_DEV= ${SENSLAB_DEV} ../../devices/cc1100_2500/libcc1100.a
V14_DEV=  ${SENSLAB_DEV} ../../devices/cc2420/libcc2420.a

## v1.3b: senslab model
wsim_senslabv13b_CFLAGS=-DMSP430f1611 -DSLABV13B -DCC1100
wsim_senslabv13b_SOURCES=senslab.c
wsim_senslabv13b_LDADD=${V13B_DEV} ${WSIMADD} ${SENSLAB_MCU}
wsim_senslabv13b_DEPENDENCIES=${wsim_senslab13b_LDADD}

## v1.4: senslab model
wsim_senslabv14_CFLAGS=-DMSP430f1611 -DSLABV14
wsim_senslabv14_SOURCES=senslab.c
wsim_senslabv14_LDADD=${V14_DEV} ${WSIMADD} ${SENSLAB_MCU}
wsim_senslabv14_DEPENDENCIES=${wsim_senslab14_LDADD}

endif
