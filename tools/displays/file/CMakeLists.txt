include_subproject(dspyutil)

include_directories(${AQSIS_TIFF_INCLUDE_DIR})

# Set up libraries and defines, depending on whether fltk is found.
set(linklibs ${AQSIS_TIFF_LIBRARIES})
if(AQSIS_USE_FLTK)
	include_directories(${AQSIS_FLTK_INCLUDE_DIR})
	list(APPEND linklibs ${AQSIS_FLTK_LIBRARIES})
	if(APPLE)
		list(APPEND linklibs ${CARBON_LIBRARY})
	endif()
else()
    add_definitions(-DAQSIS_NO_FLTK)
endif()

aqsis_add_display(file display.cpp ${dspyutil_srcs}
	LINK_LIBRARIES ${linklibs})
