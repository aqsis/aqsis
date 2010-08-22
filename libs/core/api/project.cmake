set(api_srcs
	condition.cpp
	genpoly.cpp
	graphicsstate.cpp
	ri.cpp
	rif.cpp
	../../riutil/risyms.cpp
	../../riutil/ri2ricxx.cpp
)
make_absolute(api_srcs ${api_SOURCE_DIR})

set(api_hdrs
	condition.h
	genpoly.h
	graphicsstate.h
)
make_absolute(api_hdrs ${api_SOURCE_DIR})

include_directories(${api_SOURCE_DIR})
include_directories(${api_BINARY_DIR})

set(api_test_srcs
	rif_test.cpp
)
make_absolute(api_test_srcs ${api_SOURCE_DIR})
