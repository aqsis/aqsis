# Filter out entries from a list.
MACRO(FILTER_OUT FILTERS INPUTS OUTPUT)
   # Mimicks Gnu Make's $(filter-out) which removes elements 
   # from a list that match the pattern.
   # Arguments:
   #  FILTERS - list of patterns that need to be removed
   #  INPUTS  - list of inputs that will be worked on
   #  OUTPUT  - the filtered list to be returned
   # 
   # Example: 
   #  SET(MYLIST this that and the other)
   #  SET(FILTS this that)
   #
   #  FILTER_OUT("${FILTS}" "${MYLIST}" OUT)
   #  MESSAGE("OUTPUT = ${OUT}")
   #
   # The output - 
   #   OUTPUT = and;the;other
   #
   SET(FOUT "")
   FOREACH(INP ${INPUTS})
	   SET(FILTERED 0)
	   FOREACH(FILT ${FILTERS})
		   IF(${FILTERED} EQUAL 0)
			   IF("${FILT}" STREQUAL "${INP}")
				   SET(FILTERED 1)
			   ENDIF("${FILT}" STREQUAL "${INP}")
		   ENDIF(${FILTERED} EQUAL 0)
	   ENDFOREACH(FILT ${FILTERS})
	   IF(${FILTERED} EQUAL 0)
		   SET(FOUT ${FOUT} ${INP})
	   ENDIF(${FILTERED} EQUAL 0)
   ENDFOREACH(INP ${INPUTS})
   SET(${OUTPUT} ${FOUT})
ENDMACRO(FILTER_OUT FILTERS INPUTS OUTPUT)


#----------------------------------------------------------------------
# Facilities for adding unit tests to the build

IF(AQSIS_ENABLE_TESTING)

	# AQSIS_ADD_TESTS(SOURCE_LIST LINK_LIBS)
	#
	# Adds tests based on the boost.test framework.  Each source file is
	# assumed to constitue an independent set of tests, and is built into its
	# own executable.
	#
	# Arguments:
	#   SOURCE_LIST - list of sources using boost.test.  One test set per file.
	#   LINK_LIBS   - list of libraries which these tests need to be linked against.
	#
	# The boost test library is automatically linked with the generated
	# executables, so there's no need to include it in the LINK_LIBS list.
	# 
	# The custom target all_tests allows all tests to be built at once.

	MACRO(AQSIS_ADD_TESTS SOURCE_LIST LINK_LIBS)
		# It seems strange to add all_tests it each time the macro is called...
		# perhaps there's a better way?
		ADD_CUSTOM_TARGET(all_tests)
		FOREACH(TEST_SRC ${SOURCE_LIST})
			STRING(REGEX REPLACE ".cpp$" "" TEST_NAME ${TEST_SRC})
			# Remove any prefixed-path from the test source
			STRING(REGEX REPLACE "^.*/" "" TEST_NAME ${TEST_NAME})
			ADD_EXECUTABLE(${TEST_NAME}
				${TEST_SRC}
				${CMAKE_SOURCE_DIR}/build_tools/boostautotestmain.cpp
				)
			# Perhaps we should remove boostautotestmain.cpp in favor of simply
			# defining BOOST_AUTO_TEST_MAIN in each test file.
			TARGET_LINK_LIBRARIES(${TEST_NAME}
				${LINK_LIBS}
				${AQSIS_BOOST_UNIT_TEST_FRAMEWORK_LIBRARY}
				)
			ADD_TEST("${TEST_NAME}" ${EXECUTABLE_OUTPUT_PATH}/${TEST_NAME})
			ADD_DEPENDENCIES(all_tests ${TEST_NAME})
		ENDFOREACH(TEST_SRC)
	ENDMACRO(AQSIS_ADD_TESTS)

ELSE(AQSIS_ENABLE_TESTING)

	# If testing is not enabled, AQSIS_ADD_TESTS is a No-op.
	MACRO(AQSIS_ADD_TESTS SOURCE_LIST LINK_LIBS)
	ENDMACRO(AQSIS_ADD_TESTS SOURCE_LIST LINK_LIBS)

ENDIF(AQSIS_ENABLE_TESTING)
