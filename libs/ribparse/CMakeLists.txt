project(ribparse)

# Check for boost iostreams.
if(NOT Boost_IOSTREAMS_FOUND)
	message(FATAL_ERROR "Aqsis ribparse requires boost iostreams to build")
endif()

set(ribparse_srcs
	ribinputbuffer.cpp
	ribtokenizer.cpp
	riblexer.cpp
	ribparser.cpp
	ribsema.cpp
	ribwriter.cpp
	ricxx_validate.cpp
	framedrop_filter.cpp
	ricxx_filter.cpp
)

set(ribparse_hdrs
	multistringbuffer.h
	ribinputbuffer.h
	riblexer.h
	riblexer_impl.h
	ribparser_impl.h
	ribsema.h
	ribtoken.h
	ribtokenizer.h
	ricxx.h
	ricxx2ri.h
	ricxx_filter.h
)
source_group("Header Files" FILES ${ribparse_hdrs})

aqsis_add_library(aqsis_ribparse ${ribparse_srcs} ${ribparse_hdrs}
	COMPILE_DEFINITIONS AQSIS_RIBPARSER_EXPORTS USE_GZIPPED_RIB
	LINK_LIBRARIES aqsis_riutil ${Boost_IOSTREAMS_LIBRARY} ${Boost_ZLIB_LIBRARY}
)

aqsis_install_targets(aqsis_ribparse)

#--------------------------------------------------
# Testing
set(ribparse_test_srcs
	ribinputbuffer_test.cpp
	ribtokenizer_test.cpp
	ribparser_test.cpp
	riblexer_test.cpp
	ribsema_test.cpp
	errorhandler_test.cpp
)

aqsis_add_tests(${ribparse_test_srcs} LINK_LIBRARIES aqsis_ribparse)
