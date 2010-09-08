project(core)

# Check for boost thread.
if(NOT Boost_THREAD_FOUND)
	message(FATAL_ERROR "Aqsis core requires boost thread to build")
endif()

# Generate extra files here.  Extra stuff in this directory is...
add_subproject(api)
add_subproject(ddmanager)
add_subproject(geometry)
add_subproject(raytrace)
add_subproject(texturing_old)

set(core_srcs
	attributes.cpp
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
	refcount.cpp
	occlusion.cpp
	optioncache.cpp
	options.cpp
	parameters.cpp
	renderer.cpp
	shaders.cpp
	stats.cpp
	threadscheduler.cpp
	transform.cpp
	${api_srcs}
	${ddmanager_srcs}
	${geometry_srcs}
	${raytrace_srcs}
	${texturing_old_srcs}
)

set(core_test_srcs
	${api_test_srcs}
	occlusion_test.cpp
	bilinear_test.cpp
)

set(core_hdrs
	attributes.h
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
	occlusion.h
	optioncache.h
	options.h
	parameters.h
	plane.h
	renderer.h
	shaders.h
	stats.h
	threadscheduler.h
	transform.h
	${api_hdrs}
	${ddmanager_hdrs}
	${geometry_hdrs}
	${raytrace_hdrs}
	${texturing_old_hdrs}
)

source_group("Header Files" FILES ${core_hdrs})

include_directories(
	${PROJECT_SOURCE_DIR}  #< Needed so that files in subdirectories can find headers.
	${AQSIS_TIFF_INCLUDE_DIR})

# Create list of preprocessor definitions
set(defs AQSIS_RI_EXPORTS AQSIS_CORE_EXPORTS
	"AQSIS_MAIN_CONFIG_NAME=${AQSIS_MAIN_CONFIG_NAME}")
if(AQSIS_ENABLE_MPDUMP)
	list(APPEND defs ENABLE_MPDUMP)
endif()
if(AQSIS_USE_TIMERS)
	list(APPEND defs USE_TIMERS)
endif()
if(AQSIS_ENABLE_THREADING)
	list(APPEND defs ENABLE_THREADING)
endif()
if(DEFAULT_RC_PATH)
	list(APPEND defs "DEFAULT_RC_PATH=${DEFAULT_RC_PATH}")
endif()


# Add library & install
aqsis_add_library(aqsis_core ${core_srcs} ${core_hdrs}
	TEST_SOURCES ${core_test_srcs}
	COMPILE_DEFINITIONS ${defs}
	LINK_LIBRARIES aqsis_math aqsis_riutil aqsis_shadervm
		aqsis_tex aqsis_util aqsis_riutil
		${AQSIS_TIFF_LIBRARIES} ${Boost_THREAD_LIBRARY} ${CARBON_LIBRARY}
	DEPENDS ri_inl
)

aqsis_install_targets(aqsis_core)

