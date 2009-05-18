project(doxygen_docs)

find_package(Doxygen)

if(DOXYGEN_EXECUTABLE)

#	set(AQSIS_DOXYGEN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/doxygen_docs")
#	set(AQSIS_DOC_HTML "NO")
#	set(AQSIS_DOC_SEARCHBAR "NO")
#
#	if(AQSIS_ENABLE_DOC_HTML)
#		set(AQSIS_DOC_HTML "YES")
#	endif()
#	if(AQSIS_ENABLE_DOC_SEARCHBAR)
#		set(AQSIS_DOC_SEARCHBAR "YES")
#	endif()
#
#	set(_DOXYFILE_IN  "${PROJECT_SOURCE_DIR}/Doxyfile.in.cmake")
#	set(_DOXYFILE_OUT "${PROJECT_BINARY_DIR}/Doxyfile")
#
#	# Configure the doxygen config file
#	message(STATUS "Create ${_DOXYFILE_OUT} from ${_DOXYFILE_IN}")
#	configure_file(${_DOXYFILE_IN}
#		${_DOXYFILE_OUT}
#		@ONLY)
#
#	# Setup the doc build target
#	add_custom_target(doc
#		${DOXYGEN_EXECUTABLE} ${_DOXYFILE_OUT}
#		DEPENDS ${_DOXYFILE_OUT})

endif(DOXYGEN_EXECUTABLE)
