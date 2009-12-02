if BUILD_ICLBSN

bin_PROGRAMS=wsim-iclbsn wsim-iclbsn2 wsim-iclbsn3

INCLUDES=-I$(top_srcdir)

ICLBSN_MCU1= ../../arch/msp430/libmsp430f149.a
ICLBSN_MCU2= ../../arch/msp430/libmsp430f1611.a

ICLBSN_DEV= ../../devices/led/libled.a		        \
	    ../../devices/at45db/libat45db041b.a	\
            ../../devices/ptty/libptty.a                \
            ../../devices/cc2420/libcc2420.a		\
	    ../../devices/uigfx/libuigfx.a


## v1: msp430f149
wsim_iclbsn_CFLAGS=-DMSP430f149 -DBSN1
wsim_iclbsn_SOURCES=iclbsn.c
wsim_iclbsn_LDADD=${ICLBSN_DEV} ${WSIMADD} ${ICLBSN_MCU1}

## v2: msp430f1611
wsim_iclbsn2_CFLAGS=-DMSP430f1611 -DBSN2
wsim_iclbsn2_SOURCES=iclbsn.c
wsim_iclbsn2_LDADD=${ICLBSN_DEV} ${WSIMADD} ${ICLBSN_MCU2}

## v3: msp430f1611 + SFD on P2.3
wsim_iclbsn3_CFLAGS=-DMSP430f1611 -DBSN2 -DBSN3
wsim_iclbsn3_SOURCES=iclbsn.c
wsim_iclbsn3_LDADD=${ICLBSN_DEV} ${WSIMADD} ${ICLBSN_MCU2}

endif
