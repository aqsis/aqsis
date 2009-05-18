set(api_srcs
	attributes.cpp
	condition.cpp
	genpoly.cpp
	graphicsstate.cpp
	options.cpp
	parameters.cpp
	ri.cpp
	rif.cpp
)
make_absolute(api_srcs ${api_SOURCE_DIR})

set(api_hdrs
	attributes.h
	condition.h
	genpoly.h
	graphicsstate.h
	options.h
	parameters.h
	ri_cache.h
	ri_debug.h
)
make_absolute(api_hdrs ${api_SOURCE_DIR})

include_directories(${api_SOURCE_DIR})

set(api_test_srcs
	rif_test.cpp
)
make_absolute(api_test_srcs ${api_SOURCE_DIR})
