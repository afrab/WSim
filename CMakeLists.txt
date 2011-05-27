# Copyright (C) 2005-2011 Antoine Fraboulet (http://wsim.gforge.inria.fr/)
#
# Use, modification and distribution is subject to WSIM's licensing terms
# See accompanying files LICENCE and AUTHORS for more details.

# Top-Level CmakeLists
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

PROJECT(WSIM)


# Compiler flags
IF(CMAKE_COMPILER_IS_GNUCC)
	SET(CMAKE_CXX_FLAGS "-g") # debug
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra") # warnings
ENDIF()


ADD_SUBDIRECTORY(doc)


# CPack configuration
set(CPACK_PACKAGE_NAME "wsim")

## Setting package description
set(
	CPACK_PACKAGE_DESCRIPTION
	"WorldSens Hardware Simulator"
)

set(
	CPACK_PACKAGE_DESCRIPTION_SUMMARY
	"WSim is a platform simulator. It relies on cycle accurate full platform simulation using microprocessor instruction driven timings. The simulator is able to perform a full simulation of hardware events that occur in the platform and to give back to the developer a precise timing analysis of the simulated software.

The native software of the node can be used in the simulator without the need to reconfigure or recompile the software. We use a classical GCC cross-compiler toolchain and the simulation is not attached to any particular language nor operating system. We are thus able to debug and evaluate performances of the full system at the assembly level. A precise estimation of timings, memory consumption and power can be obtained during simulation. FreeRTOS, Contiki and TinyOS operating systems have been successfully tested on simulation platforms.

The simulator can be used in standalone mode for debuging purposes when no radio device is used in the design (or when the radio simulation is not needed). But one of the main WSim feature is its interface with the WSNet simulator to perform the simulation of a complete sensor network."
)

## Setting dependancies
set(
	CPACK_DEBIAN_PACKAGE_DEPENDS
	"libc6 (>= 2.3.6), libgcc1 (>= 1:4.1)"
)


## Setting contact and vendor details
set(CPACK_PACKAGE_CONTACT "Antoine Fraboulet")
set(CPACK_PACKAGE_VENDOR "Antoine Fraboulet")

## Setting version number
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

set(CPACK_GENERATOR "DEB;RPM;")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-${CMAKE_SYSTEM_PROCESSOR}")

include(CPack)

