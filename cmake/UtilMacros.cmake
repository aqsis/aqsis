#-------------------------------------------------------------------------------
# The PARSE_ARGUMENTS macro will take the arguments of another macro and define
# several variables. The first argument to PARSE_ARGUMENTS is a prefix to put
# on all variables it creates. The second argument is a list of names, and the
# third argument is a list of options. Both of these lists should be quoted.
# The rest of the PARSE_ARGUMENTS args are arguments from another macro to be
# parsed.
# 
#     PARSE_ARGUMENTS(prefix arg_names options arg1 arg2...) 
# 
# For each item in options, PARSE_ARGUMENTS will create a variable with that
# name, prefixed with prefix_. So, for example, if prefix is MY_MACRO and
# options is OPTION1;OPTION2, then PARSE_ARGUMENTS will create the variables
# MY_MACRO_OPTION1 and MY_MACRO_OPTION2. These variables will be set to true if
# the option exists in the command line or false otherwise.
# 
# For each item in arg_names, PARSE_ARGUMENTS will create a variable with that
# name, prefixed with prefix_. Each variable will be filled with the arguments
# that occur after the given arg_name is encountered up to the next arg_name or
# the end of the arguments. All options are removed from these lists.
# PARSE_ARGUMENTS also creates a prefix_DEFAULT_ARGS variable containing the
# list of all arguments up to the first arg_name encountered. 
#
# Downloaded from: http://www.itk.org/Wiki/CMakeMacroParseArguments
#
MACRO(parse_arguments prefix arg_names option_names)
  SET(DEFAULT_ARGS)
  FOREACH(arg_name ${arg_names})    
    SET(${prefix}_${arg_name})
  ENDFOREACH(arg_name)
  FOREACH(option ${option_names})
    SET(${prefix}_${option} FALSE)
  ENDFOREACH(option)

  SET(current_arg_name DEFAULT_ARGS)
  SET(current_arg_list)
  FOREACH(arg ${ARGN})            
    SET(larg_names ${arg_names})    
    LIST(FIND larg_names "${arg}" is_arg_name)                   
    IF (is_arg_name GREATER -1)
      SET(${prefix}_${current_arg_name} ${current_arg_list})
      SET(current_arg_name ${arg})
      SET(current_arg_list)
    ELSE (is_arg_name GREATER -1)
      SET(loption_names ${option_names})    
      LIST(FIND loption_names "${arg}" is_option)            
      IF (is_option GREATER -1)
         SET(${prefix}_${arg} TRUE)
      ELSE (is_option GREATER -1)
         SET(current_arg_list ${current_arg_list} ${arg})
      ENDIF (is_option GREATER -1)
    ENDIF (is_arg_name GREATER -1)
  ENDFOREACH(arg)
  SET(${prefix}_${current_arg_name} ${current_arg_list})
ENDMACRO()



#-------------------------------------------------------------------------------
# Macro which calls the SET command if the variable to be set is currently
# empty.  In this case, "empty" means containing zero or more space characters.
#
# The arguments are identicle to the SET macro.
#
macro(set_if_empty var)
	# The "x" prefix is needed so that cmake never has to match an empty
	# string, which it seems to get upset about.
	string(REGEX REPLACE "x *" "" var_strip_space "x${${var}}")
	string(COMPARE EQUAL "${var_strip_space}" "" var_EMPTY)
	if(var_EMPTY)
		set(${var} ${ARGN})
	endif()
endmacro()


#-------------------------------------------------------------------------------
# Macro to prefix an input path with another path if the input path is not absolute.
#
# Input:
#   input_path
#   path_prefix - prefix for input_path
# Output:
#   output_path = path_prefix/input_path if input_path is relative
#               = input_path if input_path is absolute.
macro(set_with_path_prefix output_path input_path path_prefix)
	if(IS_ABSOLUTE ${input_path})
		set(${output_path} "${input_path}")
	else()
		set(${output_path} "${path_prefix}/${input_path}")
	endif()
endmacro()


#-------------------------------------------------------------------------------
# Add a prefix to all names in a list (like make's addprefix function)
#
# Usage:
#   add_prefix(var prefix name1 [name2 [name3 ...] ] )
#
# Makes "name1" into "${prefix}${name1}", etc and stores the resulting list
# into var.
macro(add_prefix var prefix)
	set(${var})
	foreach(arg ${ARGN})
		list(APPEND ${var} "${prefix}${arg}")
	endforeach()
endmacro()


#-------------------------------------------------------------------------------
# Make a list of paths absolute by prefixing with BASE_DIR if necessary
#
#   make_absolute(file_list basedir)
#
# Results in each file in file_list being prepended with ${basedir} if it's not
# absolute.  File names which are absolute are left as they are.
#
function(make_absolute file_list basedir)
	set(abs_file_list)
	foreach(src ${${file_list}})
		if(IS_ABSOLUTE ${src})
			list(APPEND abs_file_list ${src})
		else()
			list(APPEND abs_file_list "${basedir}/${src}")
		endif()
	endforeach()
	set(${file_list} ${abs_file_list} PARENT_SCOPE)
endfunction()


#-------------------------------------------------------------------------------
# Define dirname_SOURCE_DIR and dirname_BINARY_DIR, and create ${dirname_BINARY_DIR}
#
#   declare_subproject(dirname)
#
# This macro should be used in place of add_subdirectory(dirname) when
# dirname/CMakeLists.txt is only going to contain a single project(dirname)
# command.  It's designed to be used in conjunction with the include_subproject()
# macro.
#
macro(declare_subproject dirname)
	get_filename_component(_basedir_name ${dirname} NAME)
	set(${_basedir_name}_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${dirname}")
	set(${_basedir_name}_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/${dirname}")
	file(MAKE_DIRECTORY ${${_basedir_name}_BINARY_DIR})
endmacro()


#---------------
# Include dirname/project.cmake to bring the contents of the directory into the
# current cmake scope.
#
#   include_subproject(dirname)
#
# This macro is designed to be used in conjunction with add_subproject().
macro(include_subproject dirname)
	include("${${dirname}_SOURCE_DIR}/project.cmake")
endmacro()


#---------------
# Declare *and* include a subproject directory
#
macro(add_subproject dirname)
	declare_subproject(${dirname})
	include_subproject(${dirname})
endmacro()


#------------------------------------------------------------------------------
# Return the path of the current source directory relative to the base of the
# build tree.
macro(source_tree_relpath output_var)
	file(RELATIVE_PATH ${output_var}
		 ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
endmacro()

#------------------------------------------------------------------------------
# Shortcut macro for registering a library for compilation.
#
# Usage:
#
#    aqsis_add_library( target_name  source1  [source2 ...]
#                       [COMPILE_DEFINITIONS def1 ...]
#                       [DEPENDS dep1 ...]
#                       [LINK_LIBRARIES lib1 ...]
#                       [MODULE] )
#
# Specifying the PLUGIN option causes the library to be compiled as a loadable
# module (see the cmake add_library documentation regarding the MODULE library
# type).  In addition, PLUGIN causes any prefix which would normally to be
# added to the library name ("lib" on unix) to be left off.
macro(aqsis_add_library target_name)
	parse_arguments(aal "COMPILE_DEFINITIONS;DEPENDS;LINK_LIBRARIES" "PLUGIN" ${ARGN})
	set(aal_lib_type SHARED)
	if(aal_PLUGIN)
		set(aal_lib_type MODULE)
	endif()
	add_library(${target_name} ${aal_lib_type} ${aal_DEFAULT_ARGS} ${INFORES_SRCS})
	get_target_property(aal_name ${target_name} LOCATION)
	get_filename_component(aal_name ${aal_name} PATH)
	# Set the variables to be picked up if this library is needed during build, for
	# example aqsl needs aqsis_slcomp when building the shaders.
	set("${target_name}_location" ${aal_name})
	if(aal_COMPILE_DEFINITIONS)
		#message("ADDING COMPILE_DEFINITIONS: ${aal_COMPILE_DEFINITIONS}")
		set_property(TARGET ${target_name} PROPERTY
			COMPILE_DEFINITIONS ${aal_COMPILE_DEFINITIONS})
	endif()
	if(aal_DEPENDS)
		#message("ADDING DEPENDS: ${aal_DEPENDS}")
		add_dependencies(${target_name} ${aal_DEPENDS})
	endif()
	if(aal_LINK_LIBRARIES)
		#message("ADDING LINK LIBS: ${aal_LINK_LIBRARIES}")
		target_link_libraries(${target_name} ${aal_LINK_LIBRARIES})
	endif()
	if(aal_PLUGIN)
		# For plugins, leave off any standard libraray prefix.
		set_target_properties(${target_name} PROPERTIES PREFIX "")
	else()
		# Only do so-versioning for non-plugins
		set_target_properties(${target_name} PROPERTIES
			SOVERSION ${VERSION_MAJOR}
			VERSION "${VERSION_MAJOR}.${VERSION_MINOR}")
	endif()
endmacro()


#------------------------------------------------------------------------------
# Shortcut macro for registering an executable for compilation.
#
# Usage:
#
#    aqsis_add_executable( target_name  source1  [source2 ...]
#                          [GUIAPP]
#                          [COMPILE_DEFINITIONS def1 ...]
#                          [DEPENDS dep1 ...]
#                          [LINK_LIBRARIES lib1 ...] )
#
# The GUIAPP option adds some resources for icons on windows, and makes sure
# that the necessary resource fork is added on OS X.
#
macro(aqsis_add_executable target_name)
	parse_arguments(aae "COMPILE_DEFINITIONS;DEPENDS;LINK_LIBRARIES" "GUIAPP" ${ARGN})
	set(_srcs ${aae_DEFAULT_ARGS} ${INFORES_SRCS})
	if(aae_GUIAPP)
		list(APPEND _srcs ${ICONRES_SRCS})
	endif()
	add_executable(${target_name} ${_srcs})
	if(aae_COMPILE_DEFINITIONS)
		set_property(TARGET ${target_name} PROPERTY
			COMPILE_DEFINITIONS ${aae_COMPILE_DEFINITIONS})
	endif()
	if(aae_DEPENDS)
		add_dependencies(${target_name} ${aae_DEPENDS})
	endif()
	if(aae_LINK_LIBRARIES)
		target_link_libraries(${target_name} ${aae_LINK_LIBRARIES})
	endif()
	if(aae_GUIAPP)
		if(APPLE)
			# Add necessary resource fork to binaries linked against FLTK (OS X)
			add_custom_command(TARGET ${target_name}
				POST_BUILD
				COMMAND ${AQSIS_RESOURCE_EXECUTABLE}
				ARGS -t APPL -o ${target_name} ${AQSIS_FLTK_INCLUDE_DIR}/FL/mac.r
			)
			target_link_libraries(${target_name} ${CARBON_LIBRARY})
		endif()
	endif()
endmacro()

#------------------------------------------------------------------------------
# Install a set of targets to the correct directories.
#
# Usage:
#
#    aqsis_install_targets(target1 target2 ...)
#
# The targets can be any of shared or static libraries, or executables.
#
macro(aqsis_install_targets)
	install(TARGETS ${ARGN}
		RUNTIME DESTINATION ${BINDIR} COMPONENT main
		LIBRARY DESTINATION ${LIBDIR} COMPONENT main
		ARCHIVE DESTINATION ${LIBDIR} COMPONENT development)
endmacro()
