PROJECT(AQSISTYPES)

#----------------------------------------------------------------------
# create aqsis_config.h
INCLUDE (CheckIncludeFiles)

CHECK_INCLUDE_FILES(stdint.h AQSIS_HAVE_STDINT_H)
CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/aqsis_config.h.in.cmake"
	"${PROJECT_BINARY_DIR}/aqsis_config.h")


#----------------------------------------------------------------------
# Source files

# NOTE: Not sure if this best, or we should exhaustively list the files.
# The reason being, if we add a new file in this system, we need to 
# manually re-run CMake to generate the source files, however, if we
# have to modify this file then 'make' or whatever will spot it automatically
# re-run cmake.
FILE(GLOB AQSISTYPES_SRCS ${PROJECT_SOURCE_DIR}/*.cpp)
FILE(GLOB AQSISTYPES_HDRS ${PROJECT_SOURCE_DIR}/*.h)

IF(UNIX OR APPLE)
	FILE(GLOB AQSISTYPES_SYSTEM_SRCS ${PROJECT_SOURCE_DIR}/posix/*.cpp)
	FILE(GLOB AQSISTYPES_SYSTEM_HDRS ${PROJECT_SOURCE_DIR}/posix/*.h)
ELSE(UNIX OR APPLE)
	IF(WIN32)
		FILE(GLOB AQSISTYPES_SYSTEM_SRCS ${PROJECT_SOURCE_DIR}/win32/*.cpp)
		FILE(GLOB AQSISTYPES_SYSTEM_HDRS ${PROJECT_SOURCE_DIR}/win32/*.h)
	ENDIF(WIN32)
ENDIF(UNIX OR APPLE)

SET(AQSISTYPES_SRCS ${AQSISTYPES_SRCS} ${AQSISTYPES_SYSTEM_SRCS})
SET(AQSISTYPES_HDRS
	${AQSISTYPES_HDRS}
	${AQSISTYPES_SYSTEM_HDRS}
	"${PROJECT_BINARY_DIR}/aqsis_config.h"
)

FILE(GLOB AQSISTYPES_TEST_SRCS ${PROJECT_SOURCE_DIR}/*_test.cpp)

FILTER_OUT("${AQSISTYPES_TEST_SRCS}" "${AQSISTYPES_SRCS}" AQSISTYPES_SRCS) 


#------------------------------------------------------------------------------
# Targets
INCLUDE_DIRECTORIES(${AQSIS_BOOST_INCLUDE_DIR})

# Internal dependencies
INCLUDE(${PROJECT_SOURCE_DIR}/export.cmake)
INCLUDE(${RIBAPI_SOURCE_DIR}/export.cmake)

# Group header files for Visual Studio
SOURCE_GROUP("Header files" FILES ${AQSISTYPES_HDRS})

ADD_DEFINITIONS(-DCOMMON_EXPORTS)

ADD_LIBRARY(aqsistypes SHARED ${AQSISTYPES_SRCS} ${AQSISTYPES_HDRS} ${INFORES_SRCS})

GET_TARGET_PROPERTY(AQSISTYPES_LIBRARY aqsistypes LOCATION)

SET(LINKLIBS
	${AQSISTYPES_SYSTEM_LINKLIBS}
	${AQSIS_BOOST_FILESYSTEM_LIBRARY}
)
TARGET_LINK_LIBRARIES(aqsistypes ${LINKLIBS})

SET_TARGET_PROPERTIES(aqsistypes PROPERTIES OUTPUT_NAME "aqsis_types" SOVERSION ${MAJOR} VERSION "${MAJOR}.${MINOR}")

INSTALL(TARGETS aqsistypes
	RUNTIME DESTINATION ${BINDIR}
	LIBRARY DESTINATION ${LIBDIR}
	ARCHIVE DESTINATION ${LIBDIR})

INSTALL(FILES ${AQSISTYPES_HDRS} DESTINATION ${INCLUDEDIR})

#----------------------------------------------------------------------
# Testing
SET(LINKLIBS)
INCLUDE(link.cmake)
AQSIS_ADD_TESTS(${AQSISTYPES_TEST_SRCS} LINKWITH ${LINKLIBS})
