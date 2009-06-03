project(pdiff)

SET(pdiff_srcs
	PerceptualDiff.cpp
	LPyramid.cpp
	RGBAImage.cpp
	CompareArgs.cpp
	Metric.cpp
)

include_directories(${AQSIS_TIFF_INCLUDE_DIR})
include_directories(${AQSIS_ZLIB_INCLUDE_DIR})

aqsis_add_executable(pdiff ${pdiff_srcs}
	LINK_LIBRARIES ${AQSIS_TIFF_LIBRARIES} ${AQSIS_ZLIB_LIBRARIES})

aqsis_install_targets(pdiff)
