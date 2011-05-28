# Copyright (C) 2005-2011 Antoine Fraboulet (http://wsim.gforge.inria.fr/)
#
# Use, modification and distribution is subject to WSIM's licensing terms
# See accompanying files LICENCE and AUTHORS for more details.

# liblogger CMakeLists.txt

FILE(
	GLOB
	source_files
	*.c
)

FILE(
	GLOB
	header_files
	*.h
)

ADD_LIBRARY(logger STATIC ${source_files} ${header_files})

# Install the library when install target is called
# WSIM_INSTALL_TARGETS(logger)
