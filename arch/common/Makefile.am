
noinst_LIBRARIES = \
	libmcucommon.a

INCLUDES=-I$(top_srcdir)

COMMON_HEADER=          \
	debug.h    	\
	mcu.h		\
	hardware.h	\
	wsim_stdint.h

COMMON_SRC=debug.c ihex.c missing.c

libmcucommon_a_SOURCES=${COMMON_HEADER} ${COMMON_SRC}

extra_DIST=${COMMON_HEADER}
