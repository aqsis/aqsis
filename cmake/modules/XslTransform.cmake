# The following macro was borrowed from boost before extensive modification to
# work better in the context of code generation for aqsis.
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

# Transforms the source XML file by applying the given XSL stylesheet.
#
#   xsl_transform(output input
#                 STYLESHEET stylesheet
#                 [CATALOG catalog]
#                 [PARAMETERS param1=value1 param2=value2 ...]
#                 [MAKE_ALL_TARGET target])
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
# To associate a target name with the result of the XSL transformation, use the
# MAKE_ALL_TARGET option and provide the name of the target.  The target will
# be made part of the default build.
#
# The comment CMake provides when running the XSL transformation will be
# "Generating "output" via XSL transformation...".
macro(xsl_transform OUTPUT INPUT)
	parse_arguments(THIS_XSL
		"STYLESHEET;CATALOG;PARAMETERS;SEARCHPATH;MAKE_ALL_TARGET"
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
	# Actually, using find_file may be the "wrong thing to do" here, as hinted
	# by the fact that it adds stuff to the cache.  However it's easiest for now.
	find_file(_XML_LOCATION ${INPUT} PATHS ${THIS_XSL_SEARCHPATH} NO_DEFAULT_PATH)
	find_file(_STYLESHEET_LOCATION ${THIS_XSL_STYLESHEET} PATHS ${THIS_XSL_SEARCHPATH} NO_DEFAULT_PATH)
	# We don't want these variables to hang around in the cache, so remove them
	# immediately.
	unset(_XML_LOCATION CACHE)
	unset(_STYLESHEET_LOCATION CACHE)

	# Run the XSLT processor to do the XML transformation.
	get_filename_component(OUTPUT_BASENAME ${OUTPUT} NAME)
	add_custom_command(OUTPUT ${OUTPUT}
		COMMAND ${THIS_XSL_CATALOG} ${AQSIS_XSLTPROC_EXECUTABLE} ${XSLTPROC_FLAGS} 
			${THIS_XSL_EXTRA_FLAGS} -o ${OUTPUT} 
			--path ${THIS_XSL_SEARCHPATH}
			${THIS_XSL_STYLESHEET} ${INPUT}
		COMMENT "Generating ${OUTPUT_BASENAME} via XSL transformation..."
		DEPENDS ${_XML_LOCATION} ${THIS_XSL_DEFAULT_ARGS} ${_STYLESHEET_LOCATION}
	)
	set_source_files_properties(${OUTPUT} PROPERTIES GENERATED TRUE)

	# Create a custom target to refer to the result of this
	# transformation.
	if(THIS_XSL_MAKE_ALL_TARGET)
		add_custom_target(${THIS_XSL_MAKE_ALL_TARGET} ALL DEPENDS ${OUTPUT})
	endif(THIS_XSL_MAKE_ALL_TARGET)

endmacro(xsl_transform)

