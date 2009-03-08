PROJECT(doxygen_docs)

INCLUDE(FindDoxygen)

# Only try to build docs if we found doxygen installed
IF(DOXYGEN_EXECUTABLE)

	SET(AQSIS_DOXYGEN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/doxygen_docs")

	SET(__DOXYFILE_IN  "${PROJECT_SOURCE_DIR}/Doxyfile.in.cmake")
	SET(__DOXYFILE_OUT "${PROJECT_BINARY_DIR}/Doxyfile")

	# Configure the doxygen config file
	MESSAGE(STATUS "Create ${__DOXYFILE_OUT} from ${__DOXYFILE_IN}")
	CONFIGURE_FILE(${__DOXYFILE_IN}
				   ${__DOXYFILE_OUT}
				   @ONLY)

	# Setup the doc build target
	ADD_CUSTOM_TARGET(doc
					  ${DOXYGEN_EXECUTABLE} ${__DOXYFILE_OUT}
					  DEPENDS ${__DOXYFILE_OUT})

ENDIF(DOXYGEN_EXECUTABLE)
