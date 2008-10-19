PROJECT(hairgen)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6)

# Requires aqsis-1.5 to build.

SET(AQSIS_HOME "/usr/local"
	CACHE PATH
	"Base location of aqsis install")
SET(AQSIS_INCLUDE_PATH "${AQSIS_HOME}/include/aqsis"
	CACHE PATH
	"Location of aqsis includes")
SET(AQSIS_LIB_PATH "${AQSIS_HOME}/lib"
	CACHE PATH
	"Location of aqsis libraries")

SET(HAIRGEN_SOURCES
	emitter.cpp
	hair.cpp
	parenthairs.cpp
	kdtree/kdtree2.cpp
)

INCLUDE_DIRECTORIES("${AQSIS_INCLUDE_PATH}")
LINK_DIRECTORIES("${AQSIS_LIB_PATH}")

ADD_LIBRARY(hairgen SHARED ${HAIRGEN_SOURCES})

TARGET_LINK_LIBRARIES(hairgen aqsistypes ribparse)
SET_TARGET_PROPERTIES(hairgen PROPERTIES PREFIX "")
