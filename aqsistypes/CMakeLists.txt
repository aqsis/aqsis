PROJECT(AQSISTYPES)

# NOTE: Not sure if this best, or we should exhaustively list the files.
# The reason being, if we add a new file in this system, we need to 
# manually re-run CMake to generate the source files, however, if we
# have to modify this file then 'make' or whatever will spot it automatically
# re-run cmake.
FILE(GLOB AQSISTYPES_SRCS ${PROJECT_SOURCE_DIR}/*.cpp)
FILE(GLOB AQSISTYPES_HDRS ${PROJECT_SOURCE_DIR}/*.h)
# TODO: choose the right subdirectory
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
SET(AQSISTYPES_HDRS ${AQSISTYPES_HDRS} ${AQSISTYPES_SYSTEM_HDRS})

FILE(GLOB AQSISTYPES_TEST_SRCS ${PROJECT_SOURCE_DIR}/*_test.cpp)

FILTER_OUT("${AQSISTYPES_TEST_SRCS}" "${AQSISTYPES_SRCS}" AQSISTYPES_SRCS) 

INCLUDE_DIRECTORIES(${BOOST_INCLUDE_DIR})

# Internal dependencies
INCLUDE(${PROJECT_SOURCE_DIR}/export.cmake)
INCLUDE(${RIBAPI_SOURCE_DIR}/export.cmake)

# Group header files for Visual Studio
SOURCE_GROUP("Header files" FILES ${AQSISTYPES_HDRS})

# TODO: Need to do this external to this file.
IF(WIN32)
	ADD_DEFINITIONS(-DCOMMON_EXPORTS)
ENDIF(WIN32)

ADD_LIBRARY(aqsistypes SHARED ${AQSISTYPES_SRCS})
IF(WIN32)
	TARGET_LINK_LIBRARIES(aqsistypes ws2_32)
ENDIF(WIN32)
