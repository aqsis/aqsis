set(dspyutil_srcs
	dspyhlpr.c
	dspyhlpr.h
)
make_absolute(dspyutil_srcs ${dspyutil_SOURCE_DIR})

include_directories(${dspyutil_SOURCE_DIR})
