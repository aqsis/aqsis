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


##-------------------------------------------------------------------------------
## Function to get source lists from a sub-project and make sure any generated
## files are properly declared.
##
##   get_subproject(dirname)
##
## Extracts the variables dirname_srcs and dirname_hdrs present in
## ${dirname_SOURCE_DIR}/CMakeLists.txt.  Any source files in these variables
## which are specified with relative rather than absolute paths are made
## absolute.  If dirname_hdrs is nonempty, include_directories will be used
## to point the compiler in the direction of the directory containing the
## headers.
##
## To deal correctly with build-time generated source files, the user should
## declare any generated files in the variables dirname_gen_srcs and
## dirname_gen_hdrs.  If this is done, the GENERATED property will be set
## correctly for these files, and the include path updated to point at
## ${dirname_BINARY_DIR} if needed for generated headers.
#function(get_subproject subproj_name)
#	# Function to fixup any relative paths in a subproject and make them
#	# absolute.
#	function(make_subproj_paths_absolute outvar subproj_name)
#		set(abs_paths)
#		foreach(src ${ARGN})
#			if(IS_ABSOLUTE ${src})
#				list(APPEND abs_paths ${src})
#			else()
#				list(APPEND abs_paths "${${subproj_name}_SOURCE_DIR}/${src}")
#			endif()
#		endforeach()
#		set(${outvar} ${abs_paths} PARENT_SCOPE)
#	endfunction()
#	set(dir_name ${${subproj_name}_SOURCE_DIR})
#	# Find out whether a source variable is defined in the directory.  If
#	# found, define a variable in the current context corresponding to it.
#	set(srcs_varname "${subproj_name}_srcs")
#	get_directory_property(subdir_srcs DIRECTORY ${dir_name}
#		DEFINITION ${srcs_varname})
#	if(NOT subdir_srcs STREQUAL "")
#		make_subproj_paths_absolute(subdir_srcs ${subproj_name} ${subdir_srcs})
#		set(${srcs_varname} ${subdir_srcs} PARENT_SCOPE)
#	endif()
#	# Grab header files from the directory, and add the directory as an
#	# include directory if necessary.
#	set(hdrs_varname "${subproj_name}_hdrs")
#	get_directory_property(subdir_hdrs DIRECTORY ${dir_name}
#		DEFINITION ${hdrs_varname})
#	if(NOT subdir_hdrs STREQUAL "")
#		include_directories(${${subproj_name}_SOURCE_DIR})
#		make_subproj_paths_absolute(subdir_hdrs ${subproj_name} ${subdir_hdrs})
#		set(${hdrs_varname} ${subdir_hdrs} PARENT_SCOPE)
#	endif()
#	# Sanity check - make sure either sources or headers are defined in the
#	# directory.
#	if(subdir_srcs STREQUAL "" AND subdir_hdrs STREQUAL "")
#		message(SEND_ERROR "No sources or headers defined for directory ${dir_name}")
#	endif()
#	# Find out whether there are any generated files from the directory; if
#	# so, make sure they're marked as generated.
#	get_directory_property(gen_srcs DIRECTORY ${dir_name}
#		DEFINITION "${subproj_name}_gen_srcs")
#	if(NOT gen_srcs STREQUAL "")
#		set_source_files_properties(${gen_srcs} PROPERTIES GENERATED ON)
#	endif()
#	# Finally, find out whether there are any generated header files in the
#	# directory; if so, make sure they're marked as generated, and add the
#	# appropriate include directory.
#	get_directory_property(gen_hdrs DIRECTORY ${dir_name}
#		DEFINITION "${subproj_name}_gen_hdrs")
#	if(NOT gen_hdrs STREQUAL "")
#		set_source_files_properties(${gen_hdrs} PROPERTIES GENERATED ON)
#		include_directories(${${subproj_name}_BINARY_DIR})
#	endif()
#endfunction()


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
	set(${dirname}_SOURCE_DIR "${PROJECT_SOURCE_DIR}/${dirname}")
	set(${dirname}_BINARY_DIR "${PROJECT_BINARY_DIR}/${dirname}")
	file(MAKE_DIRECTORY ${${dirname}_BINARY_DIR})
endmacro()

# Include dirname/project.cmake to bring the contents of the directory into the
# current cmake scope.
#
#   include_subproject(dirname)
#
# This macro is designed to be used in conjunction with add_subproject().
macro(include_subproject dirname)
	include("${${dirname}_SOURCE_DIR}/project.cmake")
endmacro()

macro(add_subproject dirname)
	declare_subproject(${dirname})
	include_subproject(${dirname})
endmacro()


#-------------------------------------------------------------------------------
macro(aqsis_set_lib_version target_name)
	set_target_properties(${target_name} PROPERTIES 
		SOVERSION ${VERSION_MAJOR}
		VERSION "${VERSION_MAJOR}.${VERSION_MINOR}")
endmacro()


macro(aqsis_add_library target_name)
	parse_arguments(aal "COMPILE_DEFINITIONS;DEPENDS;LINK_LIBRARIES" "" ${ARGN})
	add_library(${target_name} SHARED ${aal_DEFAULT_ARGS})
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
	set_target_properties(${target_name} PROPERTIES 
		SOVERSION ${VERSION_MAJOR}
		VERSION "${VERSION_MAJOR}.${VERSION_MINOR}")
endmacro()


macro(aqsis_install_targets)
	install(TARGETS ${ARGN}
		RUNTIME DESTINATION ${BINDIR}
		LIBRARY DESTINATION ${LIBDIR}
		ARCHIVE DESTINATION ${LIBDIR})
endmacro()
