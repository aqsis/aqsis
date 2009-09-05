project(ribparse)

# Check for boost iostreams.
if(NOT Boost_IOSTREAMS_FOUND)
	message(FATAL_ERROR "Aqsis ribparse requires boost iostreams to build")
endif()

set(ribparse_srcs
	ribinputbuffer.cpp
	riblexer.cpp
	ribparser.cpp
)

set(ribparse_hdrs
	ribinputbuffer.h
	riblexer.h
	ribparser_impl.h
	ribtoken.h
)
source_group("Header Files" FILES ${ribparse_hdrs})

aqsis_add_library(aqsis_ribparse ${ribparse_srcs} ${ribparse_hdrs}
	COMPILE_DEFINITIONS AQSIS_RIBPARSER_EXPORTS USE_GZIPPED_RIB
	LINK_LIBRARIES aqsis_riutil ${Boost_IOSTREAMS_LIBRARY}
)

aqsis_install_targets(aqsis_ribparse)

#--------------------------------------------------
# Testing
set(ribparse_test_srcs
	ribinputbuffer_test.cpp
	riblexer_test.cpp
	ribparser_test.cpp
)

aqsis_add_tests(${ribparse_test_srcs} LINK_LIBRARIES aqsis_ribparse)
