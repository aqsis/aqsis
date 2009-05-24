set(raytrace_srcs
	raytrace.cpp
	raytrace.h
)
make_absolute(raytrace_srcs ${raytrace_SOURCE_DIR})

set(raytrace_hdrs
	iraytrace.h
)
make_absolute(raytrace_hdrs ${raytrace_SOURCE_DIR})

include_directories(${raytrace_SOURCE_DIR})

