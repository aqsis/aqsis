project(ptview)

find_package(OpenGL)

if(QT_FOUND AND OPENGL_FOUND)
	include_directories(${AQSIS_OPENEXR_INCLUDE_DIR} "${AQSIS_OPENEXR_INCLUDE_DIR}/OpenEXR" ${QT_INCLUDES}
						${OPENGL_INCLUDE_DIR})

	include_subproject(pointrender)

	qt4_wrap_cpp(moc_srcs
		ptview.h
		${aqsis_all_SOURCE_DIR}/include/aqsis/util/interactivecamera.h
	)
	set(srcs
		${pointrender_srcs}
		${moc_srcs}
		ptview.cpp
	)
	
	if(WIN32 AND NOT MINGW)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
	endif()
	
	set(ptview_link_libraries ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}
			${QT_QTOPENGL_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY}
			${OPENGL_gl_LIBRARY} ${pointrender_libs} aqsis_util)
			
	if(MINGW)
		list(APPEND ptview_link_libraries pthread)
	endif()
	if(WIN32)
		list(APPEND ptview_link_libraries ${QT_QTMAIN_LIBRARY})
	endif()
		
	aqsis_add_executable(ptview ${srcs} GUIAPP
		LINK_LIBRARIES  ${ptview_link_libraries})

	aqsis_install_targets(ptview)
endif()

