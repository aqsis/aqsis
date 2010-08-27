project(documentation)

#source_tree_relpath(docs_reldir)
#set(docs_install_dir ${share_install_dir}/${docs_reldir})
set(docs_install_dir ${DOCSDIR})

find_package(Doxygen)

if(DOXYGEN_EXECUTABLE)

	set(doxyfile_in  "${PROJECT_SOURCE_DIR}/manual/dev/Doxyfile.in.cmake")
	set(doxyfile_out "${PROJECT_BINARY_DIR}/manual/dev/Doxyfile")

	# Create the doxygen config file
	message(STATUS "Create ${doxyfile_out} from ${doxyfile_in}")
	configure_file(${doxyfile_in} ${doxyfile_out} @ONLY)

	# Setup the doc build target
	add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${doxyfile_out})

endif(DOXYGEN_EXECUTABLE)

install(DIRECTORY guide manual
	DESTINATION ${docs_install_dir}
	COMPONENT docs
	FILES_MATCHING PATTERN "*.pdf")

