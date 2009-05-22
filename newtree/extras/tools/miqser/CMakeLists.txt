project(miqser)

include_subproject(rib2ri)

set(miqser_srcs
	miqser.cpp
	${rib2ri_srcs}
)

set(miqser_hdrs
	${rib2ri_hdrs}
)
source_group("Header Files" FILES ${miqser_hdrs})

aqsis_add_executable(miqser ${miqser_srcs} ${miqser_hdrs}
    LINK_LIBRARIES aqsis_util aqsis_ribparse aqsis_ri2rib)

aqsis_install_targets(miqser)
