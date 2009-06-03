if BUILD_MCUGEN

noinst_LIBRARIES = libmcugen.a

INCLUDES=-I$(top_srcdir)

MCUGEN= mcugen.h mcugen.c	\
	mcugen_mac.c mcugen_jtag.c

libmcugen_a_CFLAGS=-DMCUGEN
libmcugen_a_SOURCES=${MCUGEN}

endif
