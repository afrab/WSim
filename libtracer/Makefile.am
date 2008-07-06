
noinst_LIBRARIES = libtracer.a
if BUILD_WORLDSENS
noinst_LIBRARIES += libtracerw.a
endif

INCLUDES=-I$(top_srcdir)

libtracer_a_CFLAGS=$(TRCFLAGS)
libtracer_a_SOURCES=tracer.c tracer.h

libtracerw_a_CFLAGS=$(TRCFLAGS) -DWORLDSENS
libtracerw_a_SOURCES=tracer.c tracer.h

