project(aqsl)

set(aqsl_srcs aqsl.cpp)

aqsis_add_executable(aqsl ${aqsl_srcs}
	LINK_LIBRARIES ${Boost_WAVE_LIBRARY} ${Boost_FILESYSTEM_LIBRARY}
	${Boost_THREAD_LIBRARY} aqsis_util aqsis_slcomp)

aqsis_install_targets(aqsl COMPONENT ${BINDIR})
