include_subproject(dspyutil)

include_directories(${AQSIS_TIFF_INCLUDE_DIR})

aqsis_add_display(file display.cpp ${dspyutil_srcs}
	LINK_LIBRARIES ${linklibs} ${AQSIS_TIFF_LIBRARIES})
