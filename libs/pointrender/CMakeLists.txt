project(pointrender)

find_package(Qt4 COMPONENTS QtGui QtOpenGL)

if(AQSIS_USE_OPENEXR AND QT4_FOUND)
	include_directories(${AQSIS_OPENEXR_INCLUDE_DIR} ${QT_INCLUDES})

	qt4_wrap_cpp(moc_srcs ptview.h)

	set(srcs
		${moc_srcs}
		cornellbox.cpp
		ptview.cpp
		microbuffer.cpp
	)

	add_executable(ptview ${srcs})
	target_link_libraries(ptview ${QT_QTGUI_LIBRARY} ${QT_QTOPENGL_LIBRARY}
						  ${Boost_PROGRAM_OPTIONS_LIBRARY} aqsis_tex)
endif()

