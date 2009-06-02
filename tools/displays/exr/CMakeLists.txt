include_subproject(dspyutil)

include_directories(${AQSIS_OPENEXR_INCLUDE_DIR})

aqsis_add_display(exr d_exr.cpp ${dspyutil_srcs}
	LINK_LIBRARIES ${AQSIS_OPENEXR_LIBRARIES} ${AQSIS_ZLIB_LIBRARIES})
