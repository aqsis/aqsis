project(riutil)

# Check for boost iostreams.
if(NOT Boost_IOSTREAMS_FOUND)
	message(FATAL_ERROR "Aqsis riutil requires boost iostreams to build")
endif()

set(riutil_srcs
	framedrop_filter.cpp
	renderutil_filter.cpp
	tee_filter.cpp
	primvartoken.cpp
	ribinputbuffer.cpp
	riblexer.cpp
	ribparser.cpp
	ribtokenizer.cpp
	ribwriter.cpp
	ricxx_filter.cpp
	ricxx_validate.cpp
	tokendictionary.cpp
)

set(riutil_test_srcs
	errorhandler_test.cpp
	primvartoken_test.cpp
	ribinputbuffer_test.cpp
	riblexer_test.cpp
	ribparser_test.cpp
	ribtokenizer_test.cpp
)

set(riutil_hdrs
	errorhandlerimpl.h
	multistringbuffer.h
	ribinputbuffer.h
	riblexer.h
	riblexer_impl.h
	ribparser_impl.h
	ribtoken.h
	ribtokenizer.h
	ricxx2ri.h
	ricxx_cache.h
)
source_group("Header Files" FILES ${riutil_hdrs})

aqsis_add_library(aqsis_riutil ${riutil_srcs} ${riutil_hdrs}
	TEST_SOURCES ${riutil_test_srcs}
	COMPILE_DEFINITIONS AQSIS_RIUTIL_EXPORTS USE_GZIPPED_RIB
	LINK_LIBRARIES aqsis_util ${Boost_IOSTREAMS_LIBRARY} ${AQSIS_ZLIB_LIBRARIES}
)

aqsis_install_targets(aqsis_riutil)
