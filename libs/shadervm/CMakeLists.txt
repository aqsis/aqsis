project(shadervm)

set(shadervm_srcs
	dsoshadeops.cpp
	shaderstack.cpp
	shadervm.cpp
	shadervm1.cpp
	shadervm2.cpp
)

set(shadervm_hdrs
	dsoshadeops.h
	idsoshadeops.h
	shadeopmacros.h
	shaderstack.h
	shadervariable.h
	shadervm.h
	shadervm_common.h
)
source_group("Header Files" FILES ${shadervm_hdrs})

add_subproject(shaderexecenv)

aqsis_add_library(aqsis_shadervm ${shadervm_srcs} ${shadervm_hdrs}
	${shaderexecenv_srcs} ${shaderexecenv_hdrs}
	COMPILE_DEFINITIONS AQSIS_SHADERVM_EXPORTS
	LINK_LIBRARIES aqsis_math aqsis_util aqsis_tex
)

aqsis_install_target(aqsis_shadervm)

