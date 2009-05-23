include_subproject(dspyutil)

include_directories(${AQSIS_TIFF_INCLUDE_DIR})

if(AQSIS_USE_FLTK)
    include_directories(${AQSIS_FLTK_INCLUDE_DIR})
else()
    add_definitions(-DAQSIS_NO_FLTK)
endif()

aqsis_add_display(file_display display.cpp ${dspyutil_srcs}
	LINK_LIBRARIES ${AQSIS_TIFF_LIBRARIES} ${AQSIS_FLTK_LIBRARIES})
