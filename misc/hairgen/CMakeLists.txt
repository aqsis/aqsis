PROJECT(hairgen)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6)

# Requires aqsis-1.5 to build.

SET(AQSIS_HOME "/usr"
	CACHE PATH
	"Base location of aqsis install")

SET(HAIRGEN_SOURCES
	emitter.cpp
	hair.cpp
	parenthairs.cpp
)

INCLUDE_DIRECTORIES("${AQSIS_HOME}/include")
LINK_DIRECTORIES("${AQSIS_HOME}/lib")

ADD_LIBRARY(hairgen SHARED ${HAIRGEN_SOURCES})

TARGET_LINK_LIBRARIES(hairgen aqsistypes ribparse)
SET_TARGET_PROPERTIES(hairgen PROPERTIES PREFIX "")
