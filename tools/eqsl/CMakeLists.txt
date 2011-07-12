project(eqsl)

# FIXME: Should do this in the main CmakeLists.txt
find_package(Qt4 COMPONENTS QtGui)

# Check for boost thread.
if(NOT Boost_THREAD_FOUND OR NOT Boost_REGEX_FOUND OR NOT QT4_FOUND)
	message(FATAL_ERROR "Eqsl requires boost thread, regex, and qt4 to build")
endif()

qt4_wrap_cpp(moc_srcs eqsl.h)
set(eqsl_srcs
	eqsl.cpp
	${moc_srcs}
)

if(WIN32 AND NOT MINGW)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

include_directories(${QT_INCLUDES})

aqsis_add_executable(eqsl ${eqsl_srcs} GUIAPP
	LINK_LIBRARIES aqsis_util ${QT_QTGUI_LIBRARY}
		${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY})

aqsis_install_targets(eqsl)
