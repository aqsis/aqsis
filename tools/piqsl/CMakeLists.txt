project(piqsl)

include_subproject(tinyxml)

#fltk_wrap_ui(piqsl piqsl_ui.fl)
set(piqsl_srcs
	book.cpp
	bookbrowser.cpp
	displayserverimage.cpp
	image.cpp
	piqsl.cpp
	piqslmainwindow.cpp
	${piqsl_FLTK_UI_SRCS} 
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

# The following are needed so that the generated files in
# ${piqsl_FLTK_UI_SRCS} can find the non-generated sources and vice versa.
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_BINARY_DIR})

aqsis_add_executable(piqsl ${piqsl_srcs} ${piqsl_hdrs} GUIAPP
    LINK_LIBRARIES aqsis_util aqsis_tex ${AQSIS_FLTK_LIBRARIES}
		${Boost_THREAD_LIBRARY} ${AQSIS_TINYXML_LIBRARY})

aqsis_install_target(piqsl)
