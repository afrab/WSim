
bin_PROGRAMS=wsnet1

INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/models -I$(top_srcdir)/../.. 

TRACER=$(top_srcdir)/../../libtracer/tracer.c
LOGGER=$(top_srcdir)/../../liblogger/logger.c

wsnet1_SOURCES=			\
	command_line.c		\
	core.c			\
	log.c			\
	antenna.c		\
	interference.c		\
	mobility.c		\
	models.c		\
	modulation.c		\
	nodes.c			\
	packets.c		\
	parse.c			\
	propagation.c		\
	simulation.c		\
	worldsens_srv.c		\
	worldsens_wsnet.c 	\
	$(TRACER) $(LOGGER)

wsnet1_CFLAGS=-DWSNET1

MODELS=	\
	../models/antenna/libantenna.a			\
	../models/interference/libinterference.a	\
	../models/mobility/libmobility.a		\
	../models/modulation/libmodulation.a		\
	../models/propagation/libpropagation.a

wsnet1_LDADD=$(MODELS) @EXTRALIBS@





