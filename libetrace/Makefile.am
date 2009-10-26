
noinst_LIBRARIES = libetrace.a
if BUILD_WORLDSENS
noinst_LIBRARIES += libetracew.a
endif

INCLUDES = -I$(top_srcdir)

libetrace_a_SOURCES = libetrace.h libetrace.c 

libetracew_a_CFLAGS=-DWORLDSENS
libetracew_a_SOURCES = libetrace.h libetrace.c 
