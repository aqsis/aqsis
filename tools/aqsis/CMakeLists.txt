project(aqsis)

set(aqsis_srcs
	aqsis.cpp
)

if(AQSIS_ENABLE_MPDUMP)
	add_definitions(-DENABLE_MPDUMP)
endif()

aqsis_add_executable(aqsis ${aqsis_srcs}
	LINK_LIBRARIES aqsis_core aqsis_util)

aqsis_install_targets(aqsis)
