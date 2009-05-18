project(ribparse)

set(ribparse_srcs
	iribparser.cpp
	ribinputbuffer.cpp
	riblexer.cpp
	ribparser.cpp
)

set(ribparse_hdrs
	ribinputbuffer.h
	riblexer.h
	ribparser.h
	ribtoken.h
)
source_group("Header Files" FILES ${ribparse_hdrs})

# Add aqsis_ribparse library
add_library(aqsis_ribparse SHARED ${ribparse_srcs} ${ribparse_hdrs} ${INFORES_SRCS})
set_target_properties(aqsis_ribparse PROPERTIES COMPILE_DEFINITIONS RIBPARSE_EXPORTS)
aqsis_set_lib_version(aqsis_ribparse)

# Set up linking
set(linklibs)
if(Boost_IOSTREAMS_FOUND)
	set(linklibs ${linklibs} ${Boost_IOSTREAMS_LIBRARY})
	set_target_properties(aqsis_ribparse PROPERTIES
		COMPILE_DEFINITIONS USE_GZIPPED_RIB)
else()
	message(STATUS
		"*** boost_iostreams not found ***  building without gzipped RIB support")
endif()

#include(${AQSISTYPES_SOURCE_DIR}/link.cmake)
target_link_libraries(aqsis_ribparse aqsis_riutil ${linklibs})

install(TARGETS aqsis_ribparse 
	RUNTIME DESTINATION ${BINDIR}
	LIBRARY DESTINATION ${LIBDIR}
	ARCHIVE DESTINATION ${LIBDIR})

#--------------------------------------------------
# Testing
set(ribparse_test_srcs
	ribinputbuffer_test.cpp
	riblexer_test.cpp
	ribparser_test.cpp
)

aqsis_add_tests(${ribparse_test_srcs} LINK_LIBRARIES aqsis_ribparse)
