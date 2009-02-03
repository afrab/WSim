
bin_PROGRAMS=wsnet1

INCLUDES=-I$(top_srcdir) -I$(top_srcdir)/include -I$(top_srcdir)/models

wsnet1_SOURCES=		\
	command_line.c	\
	core.c		\
	log.c		\
	antenna.c	\
	interference.c	\
	mobility.c	\
	models.c	\
	modulation.c	\
	nodes.c		\
	packets.c	\
	parse.c		\
	propagation.c	\
	simulation.c	\
	tracer.c	\
	worldsens_srv.c	\
	worldsens_wsnet.c

MODELS=	\
	../models/antenna/libantenna.a			\
	../models/interference/libinterference.a	\
	../models/mobility/libmobility.a		\
	../models/modulation/libmodulation.a		\
	../models/propagation/libpropagation.a

wsnet1_LDADD=$(MODELS) @EXTRALIBS@





