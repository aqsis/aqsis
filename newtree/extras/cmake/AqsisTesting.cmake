# Facilities for adding unit tests to the build
IF(aqsis_enable_testing)

	# Enable testing.  This causes the add_test() command to emit code.  We
	# don't really use this inbuilt facility, instead relying on the value of
	# the AQSIS_ENABLE_TESTING variable.  We just need to turn it on.
	enable_testing()


	# aqsis_add_tests(source1 [source2 ...]
	#                 [ LINK_LIBRARIES link_lib_list ]
	#                 [ EXTRASOURCE aux_source_files] )
	#
	# Adds tests based on the boost.test framework.  Each source file is
	# assumed to constitue an independent set of tests, and is built into its
	# own executable.
	#
	# Arguments:
	#   source1 [source2, ...] - set of sources using boost.test.
	#                            One test set per file.
	#   link_lib_list  - list of libraries which these tests need to be linked
	#                    against.
	#   aux_source_files - auxiliary source files which should be linked
	#                      with each of the test sources source1, ...
	#
	# The boost test library is automatically linked with the generated
	# executables, so there's no need to include it in the LINK_LIBRARIES list.
	#
	macro(aqsis_add_tests)
		parse_arguments(_atest
			"LINK_LIBRARIES;EXTRASOURCE;COMPILE_DEFINITIONS"
			""
			${ARGN}
		)
		# all source files go into _atest_DEFAULT_ARGS
		foreach(_test_src ${_atest_DEFAULT_ARGS})
			# Remove any prefixed-path from the test source and remove the
			# extension to get the test name.
			get_filename_component(TEST_EXE_NAME ${_test_src} NAME_WE)
			get_filename_component(CURR_DIR_NAME ${PROJECT_SOURCE_DIR} NAME)
			# Test name is "source_directory/file_name"
			set(TEST_NAME "${CURR_DIR_NAME}/${TEST_EXE_NAME}")
			add_executable(${TEST_EXE_NAME}
				${_test_src}
				${CMAKE_SOURCE_DIR}/libs/build_tools/boostautotestmain.cpp
				${_atest_EXTRASOURCE} )
			# Perhaps we should remove boostautotestmain.cpp in favor of simply
			# defining BOOST_AUTO_TEST_MAIN in each test file.
			target_link_libraries(${TEST_EXE_NAME}
				${_atest_LINK_LIBRARIES}
				${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
				)
			if(_atest_COMPILE_DEFINITIONS)
				set_property(TARGET ${TEST_EXE_NAME} PROPERTY
					COMPILE_DEFINITIONS ${_atest_COMPILE_DEFINITIONS})
			endif()
			add_test(${TEST_NAME} ${EXECUTABLE_OUTPUT_PATH}/${TEST_EXE_NAME})
		endforeach()
	endmacro()

else()

	# If testing is not enabled, aqsis_add_tests is a No-op.
	macro(aqsis_add_tests)
	endmacro()

endif()
