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
	ribout.cpp
	ricxx_validate.cpp
)

set(ribparse_hdrs
	ribinputbuffer.h
	ribtokenizer.h
	ribparser_impl.h
	riblexer_impl.h
	riblexer.h
	ribtoken.h
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
)

aqsis_add_tests(${ribparse_test_srcs} LINK_LIBRARIES aqsis_ribparse)

if(aqsis_enable_testing)
	# Speed test for parser
	add_executable(ricxx2ri_speed_test ricxx2ri_speed_test.cpp ricxx2ri.cpp)
	target_link_libraries(ricxx2ri_speed_test aqsis_ribparse aqsis_core)
	# Set the exports necessary to avoid incorrect linkage errors.
	set_target_properties(ricxx2ri_speed_test PROPERTIES COMPILE_DEFINITIONS AQSIS_RI_EXPORTS)

	add_executable(ribout_test ribout_test.cpp)
	target_link_libraries(ribout_test aqsis_ribparse)
endif()
