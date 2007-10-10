PROJECT(AQSISTEX)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/interfaces)
###INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/rib/api)
INCLUDE_DIRECTORIES(${AQSIS_BOOST_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${AQSIS_TIFF_INCLUDE_DIR})

# Internal dependencies
INCLUDE(${AQSISTYPES_SOURCE_DIR}/export.cmake)
INCLUDE(${DDMANAGER_SOURCE_DIR}/export.cmake)

SET(AQSISTEX_SRCS
	channellist.cpp
	itexfile.cpp
	magicnumber.cpp
	texfileheader.cpp
	tiffdirhandle.cpp
	tiffinputfile.cpp
	imagechannel.cpp
	imagebuffer.cpp
	)

ADD_DEFINITIONS(-DAQSISTEX_EXPORTS)
ADD_LIBRARY(aqsistex SHARED ${AQSISTEX_SRCS})

SET(LINKLIBS ${AQSIS_TIFF_LIBRARIES} ${AQSIS_TIFFXX_LIBRARIES})
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
	imagebuffer_test.cpp
	magicnumber_test.cpp
	texfileheader_test.cpp
	tiffdirhandle_test.cpp
	tiffinputfile_test.cpp
	)

SET(TEST_LINKLIBS  aqsistex dl ${LINKLIBS})

AQSIS_ADD_TESTS("${AQSISTEX_TEST_SRCS}" "${TEST_LINKLIBS}")

