# Generate code for RIB request handlers
set(_handler_defs_inl ${rib2ri_BINARY_DIR}/requesthandler_method_defs.inl)
set(_handler_impl_inl ${rib2ri_BINARY_DIR}/requesthandler_method_impl.inl)
set(_request_lists_inl ${rib2ri_BINARY_DIR}/requestlists.inl)

xsl_transform(${_handler_defs_inl} ri.xml STYLESHEET requesthandler_method_defs.xsl
	SEARCHPATH "${rib2ri_SOURCE_DIR}:${ri_headers_SOURCE_DIR}")
xsl_transform(${_handler_impl_inl} ri.xml STYLESHEET requesthandler_method_impl.xsl
	SEARCHPATH "${rib2ri_SOURCE_DIR}:${ri_headers_SOURCE_DIR}")
xsl_transform(${_request_lists_inl} ri.xml STYLESHEET requestlists.xsl
	SEARCHPATH "${rib2ri_SOURCE_DIR}:${ri_headers_SOURCE_DIR}")

set(rib2ri_srcs
	ribrequesthandler.cpp
	${_handler_impl_inl}
	${_request_lists_inl}
)
make_absolute(rib2ri_srcs ${rib2ri_SOURCE_DIR})

set_source_files_properties(ribrequesthandler.cpp PROPERTIES
	OBJECT_DEPENDS
	${_handler_impl_inl}
	${_request_lists_inl}
	${_handler_defs_inl}
)
# Note that there's no object file for ribrequesthandler.h, so we can't use
# OBJECT_DEPENDS to ensure requesthandler_method_defs.inl is built before using
# it.  Targets #including ribrequesthandler.h should make sure to list
# rib2ri_hdrs as part of their source list.

set(rib2ri_hdrs
	ribrequesthandler.h
	${_handler_defs_inl}
)
make_absolute(rib2ri_hdrs ${rib2ri_SOURCE_DIR})

include_directories(${rib2ri_SOURCE_DIR})
include_directories(${rib2ri_BINARY_DIR})
