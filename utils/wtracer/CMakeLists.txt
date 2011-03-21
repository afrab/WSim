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
  ${WTRACER}/src/tracer)


INCLUDE (CheckIncludeFiles)
INCLUDE (CheckTypeSize) 
CHECK_INCLUDE_FILES(strings.h HAVE_STRINGS_H) 
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)


