
noinst_LIBRARIES=libptty.a

INCLUDES=-I$(top_srcdir)

PTTY=   ptty_dev.h         \
	ptty_dev.c

libptty_a_CFLAGS=

if BUILD_PTTY
libptty_a_CFLAGS+=-DPTTY
endif

libptty_a_SOURCES=$(PTTY)

