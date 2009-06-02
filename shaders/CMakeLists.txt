project(shaders)

set(shader_dirs
	displacement
	imager
	light
	surface
	volume
)

# Make sure that all the binary directories into which the compiled shaders
# will be placed exist.
add_prefix(shader_dirs_abs "${CMAKE_CURRENT_BINARY_DIR}/" ${shader_dirs})
file(MAKE_DIRECTORY ${shader_dirs_abs})

set(compiled_shaders)
foreach(dir ${shader_dirs})
	file(GLOB shader_srcs "${dir}/*.sl")
	foreach(shader_name ${shader_srcs})
		get_filename_component(basename ${shader_name} NAME_WE)
		set(shader_output_name "${CMAKE_CURRENT_BINARY_DIR}/${dir}/${basename}.slx")
		list(APPEND compiled_shaders ${shader_output_name})
		# Add a command to compile the shader
		add_custom_command(
			OUTPUT ${shader_output_name}
			COMMAND aqsl -o ${shader_output_name}
				         -I ${CMAKE_CURRENT_SOURCE_DIR}/include ${shader_name}
			DEPENDS ${shader_name} aqsl
			COMMENT "Compiling shader ${shader_name}"
		)
	endforeach()
endforeach()

# Add a target which depends on all compiled shaders so that they'll be built
# prior to the install stage.
add_custom_target(all_shaders ALL DEPENDS ${compiled_shaders})

# Install all compiled and source shaders, and construct the installed shader
# search path.
#source_tree_relpath(shaders_reldir)
#set(shaders_install_dir ${share_install_dir}/${shaders_reldir})
set(shaders_install_dir ${SHADERDIR})
set_with_path_prefix(shaders_searchpath_dir "${SHADERDIR}" "${CMAKE_INSTALL_PREFIX}")
set(shader_search_path)
foreach(dir ${shader_dirs})
	install(
		DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${dir}"
		DESTINATION ${shaders_install_dir}
		COMPONENT shaders
	)
	install(
		DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${dir}"
		DESTINATION ${shaders_install_dir}
		COMPONENT main
	)
	set(shader_search_path
		"${shader_search_path}:${shaders_searchpath_dir}/${dir}")
endforeach()
set(shader_search_path ${shader_search_path} PARENT_SCOPE)

# Install the include directory too.
install(
	DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include"
	DESTINATION ${shaders_install_dir}
	COMPONENT shaders
)
