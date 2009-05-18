set(maketexture_srcs
	bake.cpp
	maketexture.cpp
)
make_absolute(maketexture_srcs ${maketexture_SOURCE_DIR})

set(maketexture_hdrs
	bake.h
	cachedfilter.h
	downsample.h
)
make_absolute(maketexture_hdrs ${maketexture_SOURCE_DIR})

include_directories(${maketexture_SOURCE_DIR})

