cmake_minimum_required(VERSION 2.8)

project(wtracer)


set(WSIM "../..")
set(WTRACER "${WSIM}/utils/wtracer")


include_directories(${WTRACER})
include_directories(${WTRACER}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


add_executable(wtracer.elf  \\
  ${WTRACER}/src/main       \\
  ${WTRACER}/src/drv_vcd    \\
  ${WTRACER}/src/drv_gplot  \\
  ${WTRACER}/src/drv_raw    \\
  ${WTRACER}/src/drv_sitc   \\
  ${WTRACER}/src/endian     \\
  ${WTRACER}/src/log        \\
  ${WTRACER}/src/tracer
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


