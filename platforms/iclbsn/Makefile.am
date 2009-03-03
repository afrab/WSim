if BUILD_ICLBSN

bin_PROGRAMS=wsim-iclbsn wsim-iclbsn2
if BUILD_WORLDSENS
bin_PROGRAMS+=worldsens-iclbsn worldsens-iclbsn2
endif

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

worldsens_iclbsn_CFLAGS=-DMSP430f149 -DBSN1
worldsens_iclbsn_SOURCES=iclbsn.c
worldsens_iclbsn_LDADD=${ICLBSN_DEV} ${WSIMADD_WORLDSENS} ${ICLBSN_MCU1}

## v2: msp430f1611
wsim_iclbsn2_CFLAGS=-DMSP430f1611 -DBSN2
wsim_iclbsn2_SOURCES=iclbsn.c
wsim_iclbsn2_LDADD=${ICLBSN_DEV} ${WSIMADD} ${ICLBSN_MCU2}

worldsens_iclbsn2_CFLAGS=-DMSP430f1611 -DBSN2
worldsens_iclbsn2_SOURCES=iclbsn.c
worldsens_iclbsn2_LDADD=${ICLBSN_DEV} ${WSIMADD_WORLDSENS} ${ICLBSN_MCU2}

endif
