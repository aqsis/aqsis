project(doxygen_docs)

find_package(Doxygen)

if(DOXYGEN_EXECUTABLE)

	set(doxyfile_in  "${PROJECT_SOURCE_DIR}/Doxyfile.in.cmake")
	set(doxyfile_out "${PROJECT_BINARY_DIR}/Doxyfile")

	# Create the doxygen config file
	message(STATUS "Create ${doxyfile_out} from ${doxyfile_in}")
	configure_file(${doxyfile_in} ${doxyfile_out} @ONLY)

	# Setup the doc build target
	add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${doxyfile_out})

endif(DOXYGEN_EXECUTABLE)
