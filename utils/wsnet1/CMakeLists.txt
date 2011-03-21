cmake_minimum_required(VERSION 2.8)

project(wsnet1)

set(CMAKE_C_FLAGS "-DWSNET1 -DWORLDSENS -O3 -g -Wall -Wextra -Werror -pipe")

set(WSIM "../..")
set(WSNET1 "${WSIM}/utils/wsnet1")

include_directories(${WSIM})
include_directories(${WSNET1})
include_directories(${WSNET1}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


add_executable(wsnet1.elf                     \\
        ${WSNET1}/src/core                    \\
        ${WSNET1}/src/command_line            \\
        ${WSNET1}/src/log                     \\
        ${WSNET1}/src/nodes                   \\
        ${WSNET1}/src/packets                 \\
        ${WSNET1}/src/sim_conf                \\
        ${WSNET1}/src/simulation              \\
        ${WSNET1}/src/worldsens_srv           \\
        ${WSNET1}/src/worldsens_wsnet         \\
	${WSIM}/libtracer/tracer              \\
        ${WSIM}/libtracer/tracer_vcd          \\
        ${WSIM}/libtracer/tracer_bin          \\
	${WSIM}/liblogger/logger
)


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


