project(shadervm)

# Check for boost regex.
if(NOT Boost_REGEX_FOUND)
	message(FATAL_ERROR "Aqsis shadervm requires boost regex to build")
endif()

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
	LINK_LIBRARIES aqsis_math aqsis_util aqsis_tex ${Boost_REGEX_LIBRARY}
)

aqsis_install_targets(aqsis_shadervm)

