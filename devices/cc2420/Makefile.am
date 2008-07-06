
noinst_LIBRARIES=libcc2420.a

INCLUDES=-I$(top_srcdir)

CC2420=	cc2420_dev.c	   \
	cc2420_state.c     \
	cc2420_rx.c        \
	cc2420_tx.c        \
	cc2420_registers.c \
	cc2420_strobes.c   \
	cc2420_spi.c       \
	cc2420_fifo.c      \
	cc2420_ram.c       \
	cc2420_crc_ccitt.c \
	cc2420_state.h     \
	cc2420_dev.h	   \
	cc2420.h           \
	cc2420_internals.h \
	cc2420_registers.h \
	cc2420_rx.h        \
	cc2420_spi.h       \
	cc2420_fifo.h      \
	cc2420_ram.h       \
	cc2420_crc_ccitt.h \
	cc2420_strobes.h   \
	cc2420_debug.h     

libcc2420_a_CFLAGS=
libcc2420_a_SOURCES=$(CC2420)
