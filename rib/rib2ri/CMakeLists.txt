project(RIB2RI)

include(local.cmake)

# Create generated files using xsl transformations.
#
# The MAKE_ALL_TARGET options could be removed if we produced a proper library
# here.
xsl_transform(${CMAKE_CURRENT_BINARY_DIR}/requesthandler_method_defs.inl
	api.xml
	STYLESHEET requesthandler_method_defs.xsl
	SEARCHPATH ${RIBAPI_SOURCE_DIR}
	MAKE_ALL_TARGET requesthandler_method_defs
)

xsl_transform(${CMAKE_CURRENT_BINARY_DIR}/requesthandler_method_impl.inl
	api.xml
	STYLESHEET requesthandler_method_impl.xsl
	SEARCHPATH ${RIBAPI_SOURCE_DIR}
	MAKE_ALL_TARGET requesthandler_method_impl
)

xsl_transform(${CMAKE_CURRENT_BINARY_DIR}/requestlists.inl
	api.xml
	STYLESHEET requestlists.xsl
	SEARCHPATH ${RIBAPI_SOURCE_DIR}
	MAKE_ALL_TARGET requestlists
)

# Set dependencies of source files on generated includes.

# There's no object file for ribrequesthandler.h, so it seems like the
# OBJECT_DEPENDS command won't correctly force requesthandler_method_defs.inl
# to be built before ribrequesthandler.h is used.  As a workaround it's being
# added to the dependencies for ribrequesthandler.cpp for the moment.
list(APPEND RIBREQUESTHANDLER_CPP_DEPS 
	${CMAKE_CURRENT_BINARY_DIR}/requestlists.inl
	${CMAKE_CURRENT_BINARY_DIR}/requesthandler_method_impl.inl
	${CMAKE_CURRENT_BINARY_DIR}/requesthandler_method_defs.inl
)
set_source_files_properties(ribrequesthandler.cpp PROPERTIES
	OBJECT_DEPENDS "${RIBREQUESTHANDLER_CPP_DEPS}"
)
# Add include directory for generated files.
include_directories(${RIB2RI_BINARY_DIR})

include(${AQSISTYPES_SOURCE_DIR}/export.cmake)
include(${RIBPARSE_SOURCE_DIR}/export.cmake)
include(${RIBAPI_SOURCE_DIR}/export.cmake)

set(LINKLIBS)
include(${AQSISTYPES_SOURCE_DIR}/link.cmake)
include(${RIBPARSE_SOURCE_DIR}/link.cmake)

# Note: we don't actually build a shared library here, since the files
# reference symbols which would have to be resolved at runtime.  (This is
# almost impossible on windows with DLLs and slightly messy on OSX.)  Using a
# static library as an intermediate part of the build process might be
# possible.


#--------------------------------------------------
# Unit tests

if(AQSIS_ENABLE_TESTING)
	set(RIB2RI_TEST_SRCS
		ribrequesthandler_test.cpp
	)

	aqsis_add_tests(${RIB2RI_TEST_SRCS}
		LINKWITH ${LINKLIBS}
		EXTRASOURCE ${RIB2RI_SRCS}
	)
	# Set the exports necessary to avoid incorrect linkage errors.
	set_property(TARGET ribrequesthandler_test
		PROPERTY COMPILE_DEFINITIONS RI_EXPORTS)

	# Speed test for parser and request handler
	add_executable(parser_speed_test
		parser_speed_test.cpp
		${RIB2RI_SRCS}
		)
	target_link_libraries(parser_speed_test
		${LINKLIBS}
		)
	# Set the exports necessary to avoid incorrect linkage errors.
	set_property(TARGET parser_speed_test
		PROPERTY COMPILE_DEFINITIONS RI_EXPORTS)
endif(AQSIS_ENABLE_TESTING)

