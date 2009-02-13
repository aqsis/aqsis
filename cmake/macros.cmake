#----------------------------------------------------------------------
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
#
# Macro which calls the SET command if the variable to be set is currently
# empty.  In this case, "empty" means containing zero or more space characters.
#
# The arguments are identicle to the SET macro.
#
MACRO(SET_IF_EMPTY var)
	# The "x" prefix is needed so that cmake never has to match an empty
	# string, which it seems to get upset about.
	STRING(REGEX REPLACE "x *" "" var_strip_space "x${${var}}")
	STRING(COMPARE EQUAL "${var_strip_space}" "" var_EMPTY)
	IF(var_EMPTY)
		SET(${var} ${ARGN})
	ENDIF(var_EMPTY)
ENDMACRO(SET_IF_EMPTY)


#----------------------------------------------------------------------
# Determine whether an input path is absolute.  This is a real mess; there's a
# way to fix it in cmake-2.4.8 and later using IF(IS_ABSOLUTE ...) but we don't
# want to depend on that yet.
#
# The algorithm here is roughly a clone of the version found in the C++ source
# code of cmake-2.6.0.  I'm not entirely sure if that's the right thing to do...
MACRO(CHECK_PATH_ABSOLUTE input_path path_is_absolute)
	SET(${path_is_absolute} FALSE)
	STRING(LENGTH "${input_path}" _input_length)
	IF(${_input_length} GREATER 0)
		# Note: I've left out matching paths starting with a \ on windows,
		# since I've no idea how to handle these WRT cmake string escape
		# sequences - they play terribly badly.
		IF(CYGWIN)
			# Windows with cygwin
			SET(_abs_path_regex_str "^(/|~|.:)")
		ELSEIF(WIN32)
			# Windows
			SET(_abs_path_regex_str "^(/|.:)")
		ELSE(CYGWIN)
			# Unix
			SET(_abs_path_regex_str "^(/|~)")
		ENDIF(CYGWIN)
		STRING(REGEX MATCH ${_abs_path_regex_str} _path_match "${input_path}")
		STRING(LENGTH "${_path_match}" _match_length)
		IF(${_match_length} GREATER 0)
			SET(${path_is_absolute} TRUE)
		ENDIF(${_match_length} GREATER 0)
	ENDIF(${_input_length} GREATER 0)
ENDMACRO(CHECK_PATH_ABSOLUTE)


#----------------------------------------------------------------------
# Macro to prefix an input path with another path if the input path is not absolute.
#
# Input:
#   input_path
#   path_prefix - prefix for input_path
# Output:
#   output_path = path_prefix/input_path if input_path is relative
#               = input_path if input_path is absolute.
MACRO(SET_WITH_PATH_PREFIX output_path input_path path_prefix)
	CHECK_PATH_ABSOLUTE("${input_path}" _input_path_is_absolute)
	IF(_input_path_is_absolute)
		SET(${output_path} "${input_path}")
	ELSE(_input_path_is_absolute)
		SET(${output_path} "${path_prefix}/${input_path}")
	ENDIF(_input_path_is_absolute)
ENDMACRO(SET_WITH_PATH_PREFIX)


#----------------------------------------------------------------------
# Macro to get the svn revision number
MACRO( SVN_REPOSITORY_VERSION DESTVAR TOPDIR )
	IF(NOT ${AQSIS_SVNVERSION_EXECUTABLE} STREQUAL "AQSIS_SVNVERSION_EXECUTABLE-NOTFOUND")
		EXEC_PROGRAM( ${AQSIS_SVNVERSION_EXECUTABLE} ${TOPDIR} ARGS "." OUTPUT_VARIABLE ${DESTVAR} )
	ENDIF(NOT ${AQSIS_SVNVERSION_EXECUTABLE} STREQUAL "AQSIS_SVNVERSION_EXECUTABLE-NOTFOUND")
ENDMACRO ( SVN_REPOSITORY_VERSION )


#----------------------------------------------------------------------
# Facilities for adding unit tests to the build

IF(AQSIS_ENABLE_TESTING)

	# Enable testing.  This causes the ADD_TEST command to emit code.  We don't
	# really use this inbuilt facility, instead relying on the value of the
	# AQSIS_ENABLE_TESTING variable.  We just need to turn it on.
	ENABLE_TESTING()


	# AQSIS_ADD_TESTS(source1 [source2 ...]
	#                 [ LINKWITH link_lib_list ]
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
	# executables, so there's no need to include it in the LINK_LIBS list.
	#
	MACRO(AQSIS_ADD_TESTS)
		PARSE_ARGUMENTS(THIS_ATEST
			"LINKWITH;EXTRASOURCE"
			""
			${ARGN}
		)
		# all source files go into THIS_ATEST_DEFAULT_ARGS
		FOREACH(TEST_SRC ${THIS_ATEST_DEFAULT_ARGS})
			# Remove any prefixed-path from the test source and remove the
			# extension to get the test name.
			GET_FILENAME_COMPONENT(TEST_EXE_NAME ${TEST_SRC} NAME_WE)
			GET_FILENAME_COMPONENT(CURR_DIR_NAME ${PROJECT_SOURCE_DIR} NAME)
			# Test name is "source_directory/file_name"
			SET(TEST_NAME "${CURR_DIR_NAME}/${TEST_EXE_NAME}")
			ADD_EXECUTABLE(${TEST_EXE_NAME}
				${TEST_SRC}
				${CMAKE_SOURCE_DIR}/build_tools/boostautotestmain.cpp
				${THIS_ATEST_EXTRASOURCE}
				)
			# Perhaps we should remove boostautotestmain.cpp in favor of simply
			# defining BOOST_AUTO_TEST_MAIN in each test file.
			TARGET_LINK_LIBRARIES(${TEST_EXE_NAME}
				${THIS_ATEST_LINKWITH}
				${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
				)
			ADD_TEST(${TEST_NAME} ${EXECUTABLE_OUTPUT_PATH}/${TEST_EXE_NAME})
		ENDFOREACH(TEST_SRC)
	ENDMACRO(AQSIS_ADD_TESTS)

ELSE(AQSIS_ENABLE_TESTING)

	# If testing is not enabled, AQSIS_ADD_TESTS is a No-op.
	MACRO(AQSIS_ADD_TESTS)
	ENDMACRO(AQSIS_ADD_TESTS)

ENDIF(AQSIS_ENABLE_TESTING)
