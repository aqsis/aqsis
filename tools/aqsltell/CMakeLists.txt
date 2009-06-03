project(aqsltell)

set(aqsltell_srcs
	aqsltell.cpp
)

# TODO: This definition doesn't make sense on windows!
add_definitions(-DDEFAULT_SHADER_PATH=/usr/local/aqsis/share/shaders)

aqsis_add_executable(aqsltell ${aqsltell_srcs}
    LINK_LIBRARIES aqsis_util aqsis_slxargs aqsis_tex)

# ^ TODO: aqsis_tex is needed because of the pointcloud API (ugh!)

aqsis_install_targets(aqsltell)
