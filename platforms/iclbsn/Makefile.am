if BUILD_ICLBSN

bin_PROGRAMS=wsim-iclbsn
if BUILD_WORLDSENS
bin_PROGRAMS+=worldsens-iclbsn
endif


INCLUDES=-I$(top_srcdir)

ICLBSN_MCU= ../../arch/msp430/libmsp430f149.a
ICLBSN_DEV= ../../devices/led/libled.a		        \
	    ../../devices/at45db/libat45db041b.a	\
            ../../devices/ptty/libptty.a                \
            ../../devices/cc2420/libcc2420.a


wsim_iclbsn_CFLAGS=-DMSP430f149
wsim_iclbsn_SOURCES=iclbsn.c
wsim_iclbsn_LDADD=${ICLBSN_DEV} ${WSIMADD} ${ICLBSN_MCU}

worldsens_iclbsn_CFLAGS=-DMSP430f149
worldsens_iclbsn_SOURCES=iclbsn.c
worldsens_iclbsn_LDADD=${ICLBSN_DEV} ${WSIMADD_WORLDSENS} ${ICLBSN_MCU}

endif
