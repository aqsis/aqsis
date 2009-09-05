project(aqsl)

# Check for boost thread, wave and filesystem.
if(NOT Boost_THREAD_FOUND OR NOT Boost_WAVE_FOUND OR NOT Boost_FILESYSTEM_FOUND)
	message(FATAL_ERROR "Aqsl requires boost thread, wave and filesystem to build")
endif()

set(aqsl_srcs aqsl.cpp)

aqsis_add_executable(aqsl ${aqsl_srcs}
	LINK_LIBRARIES ${Boost_WAVE_LIBRARY} ${Boost_FILESYSTEM_LIBRARY}
	${Boost_THREAD_LIBRARY} aqsis_util aqsis_slcomp)

aqsis_install_targets(aqsl)
