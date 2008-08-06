PROJECT(AQSISTEX) 
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/interfaces)
INCLUDE_DIRECTORIES(
	${AQSIS_BOOST_INCLUDE_DIR}
	${AQSIS_TIFF_INCLUDE_DIR}
	)
IF(AQSIS_USE_OPENEXR)
	INCLUDE_DIRECTORIES(${AQSIS_OPENEXR_INCLUDE_DIR})
ENDIF(AQSIS_USE_OPENEXR)

# Internal dependencies
INCLUDE(${AQSISTYPES_SOURCE_DIR}/export.cmake)
INCLUDE(${DDMANAGER_SOURCE_DIR}/export.cmake)

# Source files
SET(AQSISTEX_SRCS
	bake.cpp
	channelinfo.cpp
	channellist.cpp
	dummyenvironmentsampler.cpp
	dummytexturesampler.cpp
	ewafilter.cpp
	cachedfilter.cpp
	ienvironmentsampler.cpp
	imagechannel.cpp
	ishadowsampler.cpp
	itexinputfile.cpp
	itexoutputfile.cpp
	itexturesampler.cpp
	itiledtexinputfile.cpp
	magicnumber.cpp
	maketexture.cpp
	randomtable.cpp
	samplequad.cpp
	shadowsampler.cpp
	texfileheader.cpp
	tiffdirhandle.cpp
	tiffinputfile.cpp
	tiffoutputfile.cpp
	tiledanyinputfile.cpp
	tiledtiffinputfile.cpp
	texturecache.cpp
	texturesampleoptions.cpp
	mixedimagebuffer.cpp
	)

# Header files
SET(AQSISTEX_HDRS
	bake.h
	cachedfilter.h
	channelinfo.h
	channellist.h
	downsample.h
	dummyenvironmentsampler.h
	dummyshadowsampler.h
	dummytexturesampler.h
	cubeenvironmentsampler.h
	latlongenvironmentsampler.h
	ewafilter.h
	exrinputfile.h
	filtersupport.h
	filtertexture.h
	ienvironmentsampler.h
	iimagechannel.h
	imagechannel.h
	imagefiletype.h
	ishadowsampler.h
	itexinputfile.h
	itexoutputfile.h
	itexturemap_old.h
	itexturesampler.h
	itiledtexinputfile.h
	magicnumber.h
	maketexture.h
	mipmaplevelcache.h
	mipmaptexturesampler.h
	mixedimagebuffer.h
	randomtable.h
	sampleaccum.h
	samplequad.h
	samplevector.h
	shadowsampler.h
	texexception.h
	texfileattributes.h
	texfileheader.h
	texturebuffer.h
	texturecache.h
	texturesampleoptions.h
	tiffdirhandle.h
	tifffile_test.h
	tiffinputfile.h
	tiffoutputfile.h
	tilearray.h
	tiledanyinputfile.h
	tiledtiffinputfile.h
	wrapmode.h
	)
IF(AQSIS_USE_OPENEXR)
	SET(AQSISTEX_SRCS
		exrinputfile.cpp
		${AQSISTEX_SRCS})
ENDIF(AQSIS_USE_OPENEXR)

# Defines
IF(AQSIS_USE_OPENEXR)
	ADD_DEFINITIONS(-DUSE_OPENEXR)
ENDIF(AQSIS_USE_OPENEXR)
ADD_DEFINITIONS(-DAQSISTEX_EXPORTS)

# Add the library
ADD_LIBRARY(aqsistex SHARED ${AQSISTEX_SRCS} ${AQSISTEX_HDRS} ${INFORES_SRCS})

# Libraries to link in to aqsistex
SET(LINKLIBS ${AQSIS_TIFF_LIBRARIES}
	${AQSIS_TIFFXX_LIBRARIES}
	${AQSIS_OPENEXR_LIBRARIES})
INCLUDE(${AQSISTYPES_SOURCE_DIR}/link.cmake)
TARGET_LINK_LIBRARIES(aqsistex ${LINKLIBS})

SET_TARGET_PROPERTIES(aqsistex PROPERTIES SOVERSION ${MAJOR} VERSION "${MAJOR}.${MINOR}")

INSTALL(TARGETS aqsistex
	RUNTIME DESTINATION ${BINDIR}
	LIBRARY DESTINATION ${LIBDIR}
	ARCHIVE DESTINATION ${LIBDIR})        

SOURCE_GROUP("Header Files" FILES ${AQSISTEX_HDRS})

#----------------------------------------------------------------------
# Testing
SET(AQSISTEX_TEST_SRCS
	channellist_test.cpp
	imagechannel_test.cpp
	mixedimagebuffer_test.cpp
	magicnumber_test.cpp
	texfileheader_test.cpp
	samplequad_test.cpp
	tiffdirhandle_test.cpp
	tiffinputfile_test.cpp
	)

SET(TEST_LINKLIBS  aqsistex ${LINKLIBS})

AQSIS_ADD_TESTS("${AQSISTEX_TEST_SRCS}" "${TEST_LINKLIBS}")
