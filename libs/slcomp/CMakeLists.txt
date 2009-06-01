project(slcomp)

add_subproject(parse)
add_subproject(backend)

source_group("Header files" FILES ${parse_hdrs} ${backend_hdrs})

aqsis_add_library(aqsis_slcomp
	${parse_srcs} ${parse_hdrs}
	${backend_srcs} ${backend_hdrs}
	COMPILE_DEFINITIONS AQSIS_SLCOMP_EXPORTS
	LINK_LIBRARIES aqsis_util
)

aqsis_install_targets(aqsis_slcomp)
