set(filtering_srcs
	cachedfilter.cpp
	dummyenvironmentsampler.cpp
	dummytexturesampler.cpp
	ewafilter.cpp
	ienvironmentsampler.cpp
	iocclusionsampler.cpp
	ishadowsampler.cpp
	itexturesampler.cpp
	occlusionsampler.cpp
	randomtable.cpp
	shadowsampler.cpp
	texturecache.cpp
)
make_absolute(filtering_srcs ${filtering_SOURCE_DIR})

set(filtering_hdrs
	cubeenvironmentsampler.h
	dummyenvironmentsampler.h
	dummyocclusionsampler.h
	dummyshadowsampler.h
	dummytexturesampler.h
	ewafilter.h
	latlongenvironmentsampler.h
	mipmap.h
	occlusionsampler.h
	randomtable.h
	shadowsampler.h
	texturecache.h
	texturesampler.h
)
make_absolute(filtering_hdrs ${filtering_SOURCE_DIR})

include_directories(${filtering_SOURCE_DIR})

set(filtering_test_srcs
	samplequad_test.cpp
)
make_absolute(filtering_test_srcs ${filtering_SOURCE_DIR})
