PROJECT(RIBPARSE)

SET(RIBPARSE_HDRS
	iribparser.h
	ribinputbuffer.h
	riblexer.h
	ribparse_share.h
	ribparser.h
	ribtoken.h
)
SET(RIBPARSE_SRCS
	iribparser.cpp
	ribinputbuffer.cpp
	riblexer.cpp
	ribparser.cpp
)
SET(RIBPARSE_TEST_SRCS
	ribinputbuffer_test.cpp
	riblexer_test.cpp
	ribparser_test.cpp
)

SET(RIBPARSE_INSTALL_HDRS
	${RIBPARSE_HDRS}
)

IF(Boost_IOSTREAMS_FOUND)
	ADD_DEFINITIONS(-DUSE_GZIPPED_RIB)
ELSE(Boost_IOSTREAMS_FOUND)
	MESSAGE(STATUS "*** boost_iostreams not found ***  building without gzipped RIB support")
ENDIF(Boost_IOSTREAMS_FOUND)

INCLUDE(${AQSISTYPES_SOURCE_DIR}/export.cmake)

ADD_LIBRARY(ribparse SHARED
	${RIBPARSE_SRCS}
	${RIBPARSE_HDRS}
	${INFORES_SRCS})

# Add the export definition using target properties to 
# avoid it propagating down to the test targets.
SET_TARGET_PROPERTIES(ribparse PROPERTIES COMPILE_DEFINITIONS RIBPARSE_EXPORTS)
IF(Boost_IOSTREAMS_FOUND)
	SET(LINKLIBS ${Boost_IOSTREAMS_LIBRARY})
ENDIF(Boost_IOSTREAMS_FOUND)
INCLUDE(${AQSISTYPES_SOURCE_DIR}/link.cmake)
TARGET_LINK_LIBRARIES(ribparse ${LINKLIBS})
SET_TARGET_PROPERTIES(ribparse
	PROPERTIES 
	OUTPUT_NAME "aqsis_ribparse"
	SOVERSION ${MAJOR}
	VERSION "${MAJOR}.${MINOR}")

INSTALL(TARGETS ribparse 
	RUNTIME DESTINATION ${BINDIR}
	LIBRARY DESTINATION ${LIBDIR}
	ARCHIVE DESTINATION ${LIBDIR})

INSTALL(FILES ${RIBPARSE_HDRS} DESTINATION ${INCLUDEDIR})

SOURCE_GROUP("Header Files" FILES ${RIBPARSE_HDRS})

# Unit tests
SET(LINKLIBS)
INCLUDE(link.cmake)
AQSIS_ADD_TESTS(${RIBPARSE_TEST_SRCS} LINKWITH ${LINKLIBS})
