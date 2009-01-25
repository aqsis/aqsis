project(RIB2RI)

set(RIB2RI_HDRS
	rib2ri_share.h
	ribrequesthandler.h
)

set(RIB2RI_SRCS
	ribrequesthandler.cpp
)

# Create generated files using xsl transformations.
xsl_transform(${CMAKE_CURRENT_BINARY_DIR}/requesthandler_method_defs.inl
	api.xml
	STYLESHEET requesthandler_method_defs.xsl
	SEARCHPATH ${RIBAPI_SOURCE_DIR}
)

xsl_transform(${CMAKE_CURRENT_BINARY_DIR}/requesthandler_method_impl.inl
	api.xml
	STYLESHEET requesthandler_method_impl.xsl
	SEARCHPATH ${RIBAPI_SOURCE_DIR}
)

xsl_transform(${CMAKE_CURRENT_BINARY_DIR}/requestlists.inl
	api.xml
	STYLESHEET requestlists.xsl
	SEARCHPATH ${RIBAPI_SOURCE_DIR}
)

# Set dependencies of source files on generated includes.
set_source_files_properties(ribrequesthandler.h PROPERTIES
	OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/requesthandler_method_defs.inl
)
list(APPEND RIBREQUESTHANDLER_CPP_DEPS 
	${CMAKE_CURRENT_BINARY_DIR}/requestlists.inl
	${CMAKE_CURRENT_BINARY_DIR}/requesthandler_method_impl.inl
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

# Note: we don't actually build a library here, since the unresolved symbols
# (like RiSphereV) are problematic on windows and to a lesser extent OSX.  It
# might be possible to build a static library and link that into the other
# shared libs that need it...


#--------------------------------------------------
# Unit tests


set(RIB2RI_TEST_SRCS
	ribrequesthandler_test.cpp
)

aqsis_add_tests(${RIB2RI_TEST_SRCS}
	LINKWITH ${LINKLIBS}
	EXTRASOURCE ${RIB2RI_SRCS}
)

