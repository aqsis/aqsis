project(shaders)

include(RSLCompile)

set(shader_dirs displacement imager light surface volume)

# for visual studio IDE set the path correctly for custom commands
if(WIN32 AND MSVC_IDE)
	file(TO_NATIVE_PATH "${aqsis_util_location}" aqsis_util_path_windows)
	file(TO_NATIVE_PATH "${aqsis_slcomp_location}" aqsis_slcomp_path_windows)
	set(CMAKE_MSVCIDE_RUN_PATH "${CMAKE_MSVCIDE_RUN_PATH}" "${aqsis_util_path_windows}" "${aqsis_slcomp_path_windows}"
	CACHE STATIC "MSVC IDE Run path" FORCE)
endif()

# compile and install all shaders
set(RSL_COMPILER aqsl)
set(RSL_SETUP_VERBOSE OFF)
#
set(RSL_TARGETS)
foreach(dir ${shader_dirs})
  # setup rsl target
  file(GLOB rsl_src "${dir}/*.sl")
  set(RSL_TARGET RSL_${dir})
  add_rslshaders(${RSL_TARGET} ${rsl_src}
                 COMPILE_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/include"
                 DEPENDS aqsl)
  list(APPEND ${RSL_TARGET} ${RSL_TARGETS})

  # setup the install target for compiled shaders
  install(
    FILES ${${RSL_TARGET}_OUTPUT}
    DESTINATION ${SHADERDIR}/${dir}
    COMPONENT main)

  # setup the install target for shader src
  install(
    FILES ${rsl_src}
    DESTINATION ${SHADERDIR}/${dir}
    COMPONENT shaders)

endforeach()
add_custom_target(all_shaders ALL DEPENDS ${RSL_TARGETS})

# construct the installed shader search path
set_with_path_prefix(shaders_searchpath_dir "${SHADERDIR}" "${CMAKE_INSTALL_PREFIX}")
set(shader_search_path)
foreach(dir ${shader_dirs})
	if(shader_search_path)
		set(shader_search_path "${shader_search_path}:${shaders_searchpath_dir}/${dir}")
	else()
		set(shader_search_path "${shaders_searchpath_dir}/${dir}")
	endif()
endforeach()
set(shader_search_path ${shader_search_path} PARENT_SCOPE)

# install the include directory
install(
	DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include"
	DESTINATION ${SHADERDIR}
	COMPONENT shaders
)
