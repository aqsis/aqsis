project(ri2rib)

set(ri2rib_srcs
	ascii.cpp
	binary.cpp
	context.cpp
	error.cpp
	output.cpp
	outstream.cpp
	plstore.cpp
	ri.cpp
	tokens.cpp
)

set(ri2rib_hdrs
	ascii.h
	binary.h
	context.h
	error.h
	options.h
	output.h
	outstream.h
	plstore.h
)
source_group("Header Files" FILES ${ri2rib_hdrs})

include_directories(${AQSIS_ZLIB_INCLUDE_DIR})

aqsis_add_library(aqsis_ri2rib ${ri2rib_srcs} ${ri2rib_hdrs}
	COMPILE_DEFINITIONS AQSIS_RI_EXPORTS
	LINK_LIBRARIES aqsis_riutil ${AQSIS_ZLIB_LIBRARIES}
	DEPENDS ri_inl
)

aqsis_install_targets(aqsis_ri2rib)
