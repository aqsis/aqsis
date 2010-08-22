project(miqser)

set(miqser_srcs
	miqser.cpp
)

source_group("Header Files" FILES ${miqser_hdrs})

aqsis_add_executable(miqser ${miqser_srcs} ${miqser_hdrs}
    LINK_LIBRARIES aqsis_util aqsis_riutil)

aqsis_install_targets(miqser)
