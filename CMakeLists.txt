PROJECT(aqsis_all)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

INCLUDE(FindTIFF)
INCLUDE(FindBoost)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# TODO: find these properly
SET(FLEX_EXECUTABLE flex)
SET(BISON_EXECUTABLE bison)

IF(WIN32)
	ADD_DEFINITIONS(-DNO_SYSLOG)
	SET(LIBTIFF_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/../win32libs/libtiff/include)
ELSE(WIN32)
	ADD_DEFINITIONS(-DDEFAULT_PLUGIN_PATH="\\"/usr/local/aqsis/lib/aqsis/plugins\\"")
	ADD_DEFINITIONS(-DDEFAULT_RC_PATH="\\"/usr/local/aqsis/lib/aqsis/plugins\\"")
ENDIF(WIN32)

SET(MAJOR 1)
SET(MINOR 3)
SET(BUILD 0)
CONFIGURE_FILE(version.h.in.cmake ${PROJECT_BINARY_DIR}/version.h)

INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

SUBDIRS(argparse)
SUBDIRS(rib/api)
SUBDIRS(rib/rib2)
SUBDIRS(rib/rib2ri)
SUBDIRS(renderer/ddmanager)
SUBDIRS(renderer/raytrace)
SUBDIRS(shadercompiler/shaderexecenv)
SUBDIRS(aqsistypes)
SUBDIRS(renderer/render)
SUBDIRS(shadercompiler/shadervm)
SUBDIRS(renderer/aqsis)

