# Copyright (C) 2005-2011 Antoine Fraboulet (http://wsim.gforge.inria.fr/)
#
# Use, modification and distribution is subject to WSIM's licensing terms
# See accompanying files LICENCE and AUTHORS for more details.

# Top-Level CMakeLists.txt
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(wtracer)


# SET(WSIM "../..")
# SET(WTRACER "${WSIM}/utils/wtracer")

INCLUDE_DIRECTORIES(
	${WTRACER_SOURCE_DIR}
	${WTRACER_SOURCE_DIR}/src
	${CMAKE_CURRENT_BINARY_DIR}
)


ADD_EXECUTABLE(wtracer.elf  \\
	src/main       \\
	src/drv_vcd    \\
	src/drv_gplot  \\
	src/drv_raw    \\
	src/drv_sitc   \\
	src/endian     \\
	src/log        \\
	src/tracer
)

ADD_SUBDIRECTORY(doc)

INCLUDE (CheckIncludeFiles)
CHECK_INCLUDE_FILES(strings.h  HAVE_STRINGS_H)
CHECK_INCLUDE_FILES(inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILES(stdint.h   HAVE_STDINT_H)

INCLUDE (CheckFunctionExists) 
CHECK_FUNCTION_EXISTS(strtok_r FUNC_STRTOK_R_DEFINED) 

INCLUDE (CheckTypeSize) 
CHECK_TYPE_SIZE("char"         SIZEOF_CHAR)
CHECK_TYPE_SIZE("short"        SIZEOF_SHORT)
CHECK_TYPE_SIZE("int"          SIZEOF_INT)
CHECK_TYPE_SIZE("long"         SIZEOF_LONG)
CHECK_TYPE_SIZE("long long"    SIZEOF_LONG_LONG)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)


