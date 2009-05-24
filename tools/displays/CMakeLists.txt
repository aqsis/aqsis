declare_subproject(dspyutil)

# Helper macro to add displays targets.
macro(aqsis_add_display target_name)
	aqsis_add_library(${target_name} ${ARGN})
	get_target_property(display_lib_name ${target_name} LOCATION)
	get_filename_component(display_lib_name ${display_lib_name} NAME)
	# Set variables in the display directory to be picked up when building
	# the aqsisrc config file.
	set("${target_name}_lib_name" ${display_lib_name} PARENT_SCOPE)
	install(TARGETS ${target_name} DESTINATION ${DISPLAYSDIR})
endmacro()

add_subdirectory(exr)
add_subdirectory(file)
add_subdirectory(piqsl)
add_subdirectory(sdcBMP)
add_subdirectory(xpm)
