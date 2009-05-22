set(reyes_srcs
	bound.cpp
	bucket.cpp
	bucketprocessor.cpp
	csgtree.cpp
	filters.cpp
	grid.cpp
	imagebuffer.cpp
	imagepixel.cpp
	imagers.cpp
	lights.cpp
	micropolygon.cpp
	mpdump.cpp
	multijitter.cpp
	occlusion.cpp
	renderer.cpp
	shaders.cpp
	stats.cpp
	threadscheduler.cpp
	transform.cpp
)
make_absolute(reyes_srcs ${reyes_SOURCE_DIR})

set(reyes_hdrs
	bilinear.h
	bound.h
	bucket.h
	bucketprocessor.h
	channelbuffer.h
	clippingvolume.h
	csgtree.h
	forwarddiff.h
	grid.h
	imagebuffer.h
	imagepixel.h
	imagers.h
	isampler.h
	lights.h
	micropolygon.h
	motion.h
	mpdump.h
	multijitter.h
	objectinstance.h
	occlusion.h
	plane.h
	renderer.h
	shaders.h
	stats.h
	threadscheduler.h
	transform.h
)
make_absolute(reyes_hdrs ${reyes_SOURCE_DIR})

include_directories(${reyes_SOURCE_DIR})

set(reyes_test_srcs
	occlusion_test.cpp
)
make_absolute(reyes_test_srcs ${reyes_SOURCE_DIR})
