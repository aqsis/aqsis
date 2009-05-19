project(rib2ri)

include(project.cmake)

set(rib2ri_test_srcs
	ribrequesthandler_test.cpp
	# parser_speed_test.cpp
)

set(linklibs aqsis_ribparse aqsis_util)

aqsis_add_tests(${rib2ri_test_srcs}
	LINK_LIBRARIES ${linklibs}
	EXTRASOURCE ${rib2ri_srcs} ${rib2ri_hdrs}
	COMPILE_DEFINITIONS AQSIS_RI_EXPORTS
)

# The following is a bit of a hack, but it doesn't fit within the scope of
# aqsis_add_tests.
if(aqsis_enable_testing)
	# Speed test for parser and request handler
	add_executable(parser_speed_test parser_speed_test.cpp ${rib2ri_srcs} ${rib2ri_hdrs})
	target_link_libraries(parser_speed_test ${linklibs})
	# Set the exports necessary to avoid incorrect linkage errors.
	set_target_properties(parser_speed_test PROPERTIES COMPILE_DEFINITIONS AQSIS_RI_EXPORTS)
endif()
