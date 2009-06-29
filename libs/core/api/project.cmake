# Generate API checking and object cache code etc using XSLT.

set(_ri_cache_inl ${api_BINARY_DIR}/ri_cache.inl)
set(_ri_debug_inl ${api_BINARY_DIR}/ri_debug.inl)
set(_ri_validate_inl ${api_BINARY_DIR}/ri_validate.inl)

xsl_transform(${_ri_cache_inl} ri.xml STYLESHEET ri_cache.xsl
	SEARCHPATH "${api_SOURCE_DIR}:${ri_headers_SOURCE_DIR}")
xsl_transform(${_ri_debug_inl} ri.xml STYLESHEET ri_debug.xsl
	SEARCHPATH "${api_SOURCE_DIR}:${ri_headers_SOURCE_DIR}")
xsl_transform(${_ri_validate_inl} ri.xml STYLESHEET ri_validate.xsl
	SEARCHPATH "${api_SOURCE_DIR}:${ri_headers_SOURCE_DIR}")

set(api_srcs
	condition.cpp
	genpoly.cpp
	graphicsstate.cpp
	ri.cpp
	rif.cpp
)
make_absolute(api_srcs ${api_SOURCE_DIR})

set(api_hdrs
	condition.h
	genpoly.h
	graphicsstate.h
	ri_cache.h
	ri_debug.h
	${_ri_cache_inl}
	${_ri_debug_inl}
	${_ri_validate_inl}
)
make_absolute(api_hdrs ${api_SOURCE_DIR})

include_directories(${api_SOURCE_DIR})
include_directories(${api_BINARY_DIR})

set(api_test_srcs
	rif_test.cpp
)
make_absolute(api_test_srcs ${api_SOURCE_DIR})
