PROJECT(AQSISTEX)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/interfaces)
INCLUDE_DIRECTORIES(
	${AQSIS_BOOST_INCLUDE_DIR}
	${AQSIS_OPENEXR_INCLUDE_DIR}
	${AQSIS_TIFF_INCLUDE_DIR}
	)

# Internal dependencies
INCLUDE(${AQSISTYPES_SOURCE_DIR}/export.cmake)
INCLUDE(${DDMANAGER_SOURCE_DIR}/export.cmake)

# Source files
SET(AQSISTEX_SRCS
	channelinfo.cpp
	channellist.cpp
	itexinputfile.cpp
	itexoutputfile.cpp
	magicnumber.cpp
	texfileheader.cpp
	tiffdirhandle.cpp
	tiffinputfile.cpp
	tiffoutputfile.cpp
	texturemap2.cpp
	imagechannel.cpp
	mixedimagebuffer.cpp
	)
IF(AQSIS_OPENEXR_FOUND)
	SET(AQSISTEX_SRCS
		exrinputfile.cpp
		${AQSISTEX_SRCS})
ENDIF(AQSIS_OPENEXR_FOUND)

# Defines
IF(AQSIS_OPENEXR_FOUND)
	ADD_DEFINITIONS(-DUSE_OPENEXR)
ENDIF(AQSIS_OPENEXR_FOUND)
ADD_DEFINITIONS(-DAQSISTEX_EXPORTS)

# Add the library
ADD_LIBRARY(aqsistex SHARED ${AQSISTEX_SRCS})

# Libraries to link in to aqsistex
SET(LINKLIBS ${AQSIS_TIFF_LIBRARIES}
	${AQSIS_TIFFXX_LIBRARIES}
	${AQSIS_OPENEXR_LIBRARIES})
INCLUDE(${AQSISTYPES_SOURCE_DIR}/link.cmake)
TARGET_LINK_LIBRARIES(aqsistex ${LINKLIBS})

SET_TARGET_PROPERTIES(aqsistex PROPERTIES VERSION ${MAJOR} SOVERSION "${MAJOR}.${MINOR}")

INSTALL(TARGETS aqsistex
	RUNTIME DESTINATION ${RENDERENGINEDIR}
	LIBRARY DESTINATION ${RENDERENGINEDIR}
	ARCHIVE DESTINATION ${LIBDIR})        


#----------------------------------------------------------------------
# Testing
SET(AQSISTEX_TEST_SRCS
	channellist_test.cpp
	imagechannel_test.cpp
	mixedimagebuffer_test.cpp
	magicnumber_test.cpp
	texfileheader_test.cpp
	tiffdirhandle_test.cpp
	tiffinputfile_test.cpp
	)

SET(TEST_LINKLIBS  aqsistex dl ${LINKLIBS})

AQSIS_ADD_TESTS("${AQSISTEX_TEST_SRCS}" "${TEST_LINKLIBS}")

