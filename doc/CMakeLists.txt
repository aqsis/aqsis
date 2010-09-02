project(documentation)

#source_tree_relpath(docs_reldir)
#set(docs_install_dir ${share_install_dir}/${docs_reldir})
set(docs_install_dir ${DOCSDIR})

if(AQSIS_SPHINX_EXECUTABLE)
	add_custom_target(devguide make BUILDDIR="${PROJECT_BINARY_DIR}/guide/dev" html latexpdf
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/guide/dev")
	add_custom_target(houdiniguide make BUILDDIR="${PROJECT_BINARY_DIR}/guide/houdini" html latexpdf
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/guide/houdini")
	add_custom_target(usermanual make BUILDDIR="${PROJECT_BINARY_DIR}/manual/user" html latexpdf
		WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/manual/user")
endif()

if(DOXYGEN_EXECUTABLE)
	configure_file("${PROJECT_SOURCE_DIR}/manual/dev/Doxyfile.in.cmake" "${PROJECT_BINARY_DIR}/manual/dev/Doxyfile" @ONLY)

	add_custom_target(devmanual ${DOXYGEN_EXECUTABLE}
		WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/manual/dev")
endif()

install(DIRECTORY ${PROJECT_BINARY_DIR}/guide/dev/latex/ ${PROJECT_BINARY_DIR}/guide/houdini/latex/ ${PROJECT_BINARY_DIR}/manual/dev/latex/ ${PROJECT_BINARY_DIR}/manual/user/latex/
	DESTINATION ${docs_install_dir}
	COMPONENT docs
	FILES_MATCHING PATTERN "*.pdf")

