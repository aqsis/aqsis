#
# The following handy macros are borrowed from boost and used here with
# modifications to mesh better with the aqsis build and to fix some bugs (the
# original xsl_transform() didn't correctly generate a dependency on the xsl
# file.
#
# Original copyright message:
#
##########################################################################
# Copyright (C) 2008 Douglas Gregor <doug.gregor@gmail.com>              #
# Copyright (C) 2007 Troy Straszheim                                     #
#                                                                        #
# Distributed under the Boost Software License, Version 1.0.             #
# See http://www.boost.org/LICENSE_1_0.txt                               #
##########################################################################


# This utility macro determines whether a particular string value
# occurs within a list of strings:
#
#  list_contains(result string_to_find arg1 arg2 arg3 ... argn)
# 
# This macro sets the variable named by result equal to TRUE if
# string_to_find is found anywhere in the following arguments.
macro(list_contains var value)
  set(${var})
  foreach (value2 ${ARGN})
    if (${value} STREQUAL ${value2})
      set(${var} TRUE)
    endif (${value} STREQUAL ${value2})
  endforeach (value2)
endmacro(list_contains)


# The PARSE_ARGUMENTS macro will take the arguments of another macro and
# define several variables. The first argument to PARSE_ARGUMENTS is a
# prefix to put on all variables it creates. The second argument is a
# list of names, and the third argument is a list of options. Both of
# these lists should be quoted. The rest of PARSE_ARGUMENTS are
# arguments from another macro to be parsed.
# 
#     PARSE_ARGUMENTS(prefix arg_names options arg1 arg2...) 
# 
# For each item in options, PARSE_ARGUMENTS will create a variable with
# that name, prefixed with prefix_. So, for example, if prefix is
# MY_MACRO and options is OPTION1;OPTION2, then PARSE_ARGUMENTS will
# create the variables MY_MACRO_OPTION1 and MY_MACRO_OPTION2. These
# variables will be set to true if the option exists in the command line
# or false otherwise.
# 
# For each item in arg_names, PARSE_ARGUMENTS will create a variable
# with that name, prefixed with prefix_. Each variable will be filled
# with the arguments that occur after the given arg_name is encountered
# up to the next arg_name or the end of the arguments. All options are
# removed from these lists. PARSE_ARGUMENTS also creates a
# prefix_DEFAULT_ARGS variable containing the list of all arguments up
# to the first arg_name encountered.
MACRO(PARSE_ARGUMENTS prefix arg_names option_names)
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
    LIST_CONTAINS(is_arg_name ${arg} ${arg_names})
    IF (is_arg_name)
      SET(${prefix}_${current_arg_name} ${current_arg_list})
      SET(current_arg_name ${arg})
      SET(current_arg_list)
    ELSE (is_arg_name)
      LIST_CONTAINS(is_option ${arg} ${option_names})
      IF (is_option)
      SET(${prefix}_${arg} TRUE)
      ELSE (is_option)
      SET(current_arg_list ${current_arg_list} ${arg})
      ENDIF (is_option)
    ENDIF (is_arg_name)
  ENDFOREACH(arg)
  SET(${prefix}_${current_arg_name} ${current_arg_list})
ENDMACRO(PARSE_ARGUMENTS)


# Transforms the source XML file by applying the given XSL stylesheet.
#
#   xsl_transform(output input
#                 STYLESHEET stylesheet
#                 [CATALOG catalog]
#                 [PARAMETERS param1=value1 param2=value2 ...])
#
# This macro builds a custom command that transforms an XML file (input) via
# the given XSL stylesheet. The STYLESHEET stylesheet must be a valid XSL
# stylesheet. Any extra input files will be used as additional dependencies for
# the target. For example, these extra input files might refer to other XML
# files that are included by the input file through XInclude.
#
# When the XSL transform output is going to a directory, the mainfile
# argument provides the name of a file that will be generated within
# the output directory. This file will be used for dependency tracking.
# 
# XML catalogs can be used to remap parts of URIs within the
# stylesheet to other (typically local) entities. To provide an XML
# catalog file, specify the name of the XML catalog file via the
# CATALOG argument. It will be provided to the XSL transform.
# 
# The PARAMETERS argument is followed by param=value pairs that set
# additional parameters to the XSL stylesheet. The parameter names
# that can be used correspond to the <xsl:param> elements within the
# stylesheet.
# 
# A target name of ${STYLESHEET}__${INPUT}__result will be generated
# for the transformation, and made part of the default build.
#
# The comment CMake provides when running the XSL transformation will be
# "Generating "output" via XSL transformation...".
macro(xsl_transform OUTPUT INPUT)
	parse_arguments(THIS_XSL
		"STYLESHEET;CATALOG;PARAMETERS;SEARCHPATH"
		""
		${ARGN}
	)

	if(NOT THIS_XSL_STYLESHEET)
		message(SEND_ERROR 
		"xsl_transform macro invoked without a STYLESHEET argument")
	endif(NOT THIS_XSL_STYLESHEET)

	# TODO: Is this the best way to handle catalogs? The alternative is
	# that we could provide explicit remappings to the xsl_transform
	# macro, and it could generate a temporary XML catalog file.
	if (THIS_XSL_CATALOG)
		set(THIS_XSL_CATALOG "XML_CATALOG_FILES=${THIS_XSL_CATALOG}")
	endif (THIS_XSL_CATALOG)

	# Translate XSL parameters into a form that xsltproc can use.
	set(THIS_XSL_EXTRA_FLAGS)
	foreach(PARAM ${THIS_XSL_PARAMETERS})
		string(REGEX REPLACE "([^=]*)=([^;]*)" "\\1;\\2"
			XSL_PARAM_LIST ${PARAM})
		list(GET XSL_PARAM_LIST 0 XSL_PARAM_NAME)
		list(GET XSL_PARAM_LIST 1 XSL_PARAM_VALUE)
		list(APPEND THIS_XSL_EXTRA_FLAGS 
			--stringparam ${XSL_PARAM_NAME} ${XSL_PARAM_VALUE})
	endforeach(PARAM)

	# Find the location of the input XML file.  This is so we can set the
	# dependencies of the generated file correctly (the input XML may be in
	# a directory other than the current one).
	if(THIS_XSL_SEARCHPATH)
		set(THIS_XSL_SEARCHPATH "${CMAKE_CURRENT_SOURCE_DIR}:${THIS_XSL_SEARCHPATH}")
	else(THIS_XSL_SEARCHPATH)
		set(THIS_XSL_SEARCHPATH "${CMAKE_CURRENT_SOURCE_DIR}")
	endif(THIS_XSL_SEARCHPATH)
	find_file(INPUT_FULL_LOC ${INPUT} PATHS ${THIS_XSL_SEARCHPATH} NO_DEFAULT_PATH)

	# Run the XSLT processor to do the XML transformation.
	get_filename_component(OUTPUT_BASENAME ${OUTPUT} NAME)
	add_custom_command(OUTPUT ${OUTPUT}
		COMMAND ${THIS_XSL_CATALOG} ${AQSIS_XSLTPROC_EXECUTABLE} ${XSLTPROC_FLAGS} 
			${THIS_XSL_EXTRA_FLAGS} -o ${OUTPUT} 
			--path ${THIS_XSL_SEARCHPATH}
			${THIS_XSL_STYLESHEET} ${INPUT}
		COMMENT "Generating ${OUTPUT_BASENAME} via XSL transformation..."
		DEPENDS ${INPUT_FULL_LOC} ${THIS_XSL_DEFAULT_ARGS} ${THIS_XSL_STYLESHEET}
	)
	set_source_files_properties(${OUTPUT} PROPERTIES GENERATED TRUE)

	# Create a custom target to refer to the result of this
	# transformation.
	add_custom_target("${THIS_XSL_STYLESHEET}__${INPUT}__result" ALL
		DEPENDS ${OUTPUT})

endmacro(xsl_transform)

