include_subproject(dspyutil)

include_directories(${AQSIS_TIFF_INCLUDE_DIR})

# Set up libraries and defines, depending on whether fltk is found.
set(linklibs ${AQSIS_TIFF_LIBRARIES})
add_definitions(-DAQSIS_NO_FLTK)

aqsis_add_display(file display.cpp ${dspyutil_srcs}
	LINK_LIBRARIES ${linklibs})
