project(pointrender)

find_package(OpenGL)

if(AQSIS_USE_OPENEXR AND QT_FOUND AND OPENGL_FOUND)
	include_directories(${AQSIS_OPENEXR_INCLUDE_DIR} ${QT_INCLUDES}
						${OPENGL_INCLUDE_DIR})

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
		LINK_LIBRARIES  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}
			${QT_QTOPENGL_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY}
			${OPENGL_gl_LIBRARY} ${partio_libs} aqsis_tex)

	aqsis_install_targets(ptview)
endif()

