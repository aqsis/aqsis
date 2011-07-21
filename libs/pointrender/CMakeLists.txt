project(pointrender)

find_package(Qt4 COMPONENTS QtGui QtOpenGL)

if(AQSIS_USE_OPENEXR AND QT4_FOUND)
	include_directories(${AQSIS_OPENEXR_INCLUDE_DIR} ${QT_INCLUDES})

	qt4_wrap_cpp(moc_srcs ptview.h interactivecamera.h)

	include_subproject(partio)

	set(srcs
		${moc_srcs}
		${partio_srcs}
		ptview.cpp
		microbuffer.cpp
		pointcontainer.cpp
	)

	aqsis_add_executable(ptview ${srcs} GUIAPP
		LINK_LIBRARIES  ${QT_QTGUI_LIBRARY} ${QT_QTOPENGL_LIBRARY}
			${Boost_PROGRAM_OPTIONS_LIBRARY} ${partio_libs} aqsis_tex)

	aqsis_install_targets(ptview)
endif()

