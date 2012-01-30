project(shadervm)

# Check for boost regex.
if(NOT Boost_REGEX_FOUND OR NOT AQSIS_USE_OPENEXR)
	message(FATAL_ERROR "Aqsis shadervm requires boost regex and OpenEXR to build")
endif()

include_directories(${AQSIS_OPENEXR_INCLUDE_DIR} "${AQSIS_OPENEXR_INCLUDE_DIR}/OpenEXR")

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
include_subproject(pointrender)

set(shadervm_link_libraries aqsis_math aqsis_util aqsis_tex ${Boost_REGEX_LIBRARY} ${pointrender_libs})
if(MINGW)
 list(APPEND shadervm_link_libraries pthread)
endif()


aqsis_add_library(aqsis_shadervm ${shadervm_srcs} ${shadervm_hdrs}
	${shaderexecenv_srcs} ${shaderexecenv_hdrs} ${pointrender_srcs}
	COMPILE_DEFINITIONS AQSIS_SHADERVM_EXPORTS
	LINK_LIBRARIES ${shadervm_link_libraries}
)

aqsis_install_targets(aqsis_shadervm)

