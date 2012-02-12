project(eqsl)

# Check for boost thread.
if(NOT Boost_THREAD_FOUND OR NOT Boost_REGEX_FOUND OR NOT QT_FOUND)
	message(FATAL_ERROR "Eqsl requires boost thread, regex, and Qt4 to build")
endif()

qt4_wrap_cpp(moc_srcs eqsl.h)
set(eqsl_srcs
	eqsl.cpp
	${moc_srcs}
)

set(eql_link_libraries)

if(WIN32 AND NOT MINGW)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
	set(eqsl_link_libraries ${eqsl_link_libraries} ${QT_QTMAIN_LIBRARY})
endif()

include_directories(${QT_INCLUDES})

aqsis_add_executable(eqsl ${eqsl_srcs} GUIAPP
	LINK_LIBRARIES aqsis_util ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY}
		${eqsl_link_libraries}
		${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY})

aqsis_install_targets(eqsl)
