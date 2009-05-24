set(texturing_old_srcs
	environment_old.cpp
	shadowmap_old.cpp
	texturemap_old.cpp
)
make_absolute(texturing_old_srcs ${texturing_old_SOURCE_DIR})

set(texturing_old_hdrs
	itexturemap_old.h
	texturemap_old.h
)
make_absolute(texturing_old_hdrs ${texturing_old_SOURCE_DIR})

include_directories(${texturing_old_SOURCE_DIR})

