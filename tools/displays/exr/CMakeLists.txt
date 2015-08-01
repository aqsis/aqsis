include_subproject(dspyutil)

include_directories(${AQSIS_OPENEXR_INCLUDE_DIR} "${AQSIS_OPENEXR_INCLUDE_DIR}/OpenEXR")

if(WIN32 AND AQSIS_USE_OPENEXR_DLL)
    add_definitions(-DOPENEXR_DLL)
endif()

aqsis_add_display(exr d_exr.cpp ${dspyutil_srcs}
	LINK_LIBRARIES ${AQSIS_OPENEXR_LIBRARIES} ${AQSIS_ZLIB_LIBRARIES})
