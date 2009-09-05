project(eqsl)

# Check for boost thread.
if(NOT Boost_THREAD_FOUND OR NOT Boost_REGEX_FOUND)
	message(FATAL_ERROR "Eqsl requires boost thread and regex to build")
endif()

fltk_wrap_ui(eqsl eqsl.fl)
set(eqsl_srcs
	${eqsl_FLTK_UI_SRCS} 
)

if(WIN32)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

include_directories(${AQSIS_FLTK_INCLUDE_DIR})

aqsis_add_executable(eqsl ${eqsl_srcs} GUIAPP
	LINK_LIBRARIES aqsis_util ${AQSIS_FLTK_LIBRARIES} 
		${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY})

aqsis_install_targets(eqsl)
