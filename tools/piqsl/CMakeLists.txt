project(piqsl)

# Check for boost thread.
if(NOT Boost_THREAD_FOUND)
	message(FATAL_ERROR "Piqsl requires boost thread to build")
endif()

include_subproject(tinyxml)

set(piqsl_srcs
	book.cpp
	bookbrowser.cpp
	centerscroll.cpp
	displayserverimage.cpp
	image.cpp
	pane.cpp
	piqsl.cpp
	piqslmainwindow.cpp
	zoomimage.cpp
	${tinyxml_srcs}
)

set(piqsl_hdrs
	book.h
	bookbrowser.h
	displayserverimage.h
	image.h
	piqsl.h
	piqslmainwindow.h
	centerscroll.h
	pane.h
	zoomimage.h
	${tinyxml_hdrs}
)
source_group("Header Files" FILES ${piqsl_hdrs})

include_directories(${AQSIS_FLTK_INCLUDE_DIR})

aqsis_add_executable(piqsl ${piqsl_srcs} ${piqsl_hdrs} GUIAPP
    LINK_LIBRARIES aqsis_util aqsis_tex ${AQSIS_FLTK_LIBRARIES}
		${Boost_THREAD_LIBRARY} ${AQSIS_TINYXML_LIBRARY})

aqsis_install_targets(piqsl)
