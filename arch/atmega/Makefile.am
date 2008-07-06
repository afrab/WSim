if BUILD_ATMEGA

noinst_LIBRARIES = 		\
	libatmega128.a		\
	libatmega128l.a

INCLUDES=-I$(top_srcdir)

ATMEGA= atmega128_models.h		        \
	atmega128.h          atmega128.c        \
	atmega128_debug.h    atmega128_debug.c 	\
	atmega128_alu.h      atmega128_alu.c    \
	atmega128_io.h       atmega128_io.c     \
	atmega128_digiIO.h   atmega128_digiIO.c

libatmega128_a_CFLAGS=-DATMEGA128
libatmega128_a_SOURCES=${ATMEGA}

libatmega128l_a_CFLAGS=-DATMEGA128L
libatmega128l_a_SOURCES=${ATMEGA}

endif
