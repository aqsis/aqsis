project(RIB2RI)

set(RIB2RI_HDRS
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

add_definitions(-DRIB2RI_EXPORTS)
include(${AQSISTYPES_SOURCE_DIR}/export.cmake)
include(${RIBPARSE_SOURCE_DIR}/export.cmake)
include(${RIBAPI_SOURCE_DIR}/export.cmake)

add_library(aqsis_rib2ri SHARED
	${RIB2RI_HDRS}
	${RIB2RI_SRCS}
)

set(LINKLIBS)
include(${AQSISTYPES_SOURCE_DIR}/link.cmake)
include(${RIBPARSE_SOURCE_DIR}/link.cmake)

target_link_libraries(aqsis_rib2ri ${LINKLIBS})


#--------------------------------------------------
# Unit tests

set(RIB2RI_TEST_SRCS
	ribrequesthandler_test.cpp
)

set(LINKLIBS)
include(link.cmake)
aqsis_add_tests(${RIB2RI_TEST_SRCS} ${LINKLIBS})
