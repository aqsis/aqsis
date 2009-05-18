project(ri_headers)

set(ri_xml ${ri_headers_SOURCE_DIR}/ri.xml)

# Generate ri.inl from the XML specification.
set(ri_inl ${ri_headers_BINARY_DIR}/ri.inl)
xsl_transform(${ri_inl} ri.xml STYLESHEET ri_inl.xsl
	MAKE_ALL_TARGET ri_inl_target)

# Install all headers.
set(ri_install_hdrs
	ndspy.h
	pointcloud.h
	ri.h
	ritypes.h
	rif.h
	shadeop.h
	slo.h
	slx.h
	${ri_inl}
)

install(FILES ${ri_install_hdrs} DESTINATION ${inc_install_dir}/ri)

# FIXME
# From project.cmake:

#set(api_xml ${ri_headers_SOURCE_DIR}/ri.xml)
#
#set(_ri_inl ${ri_headers_BINARY_DIR}/ri.inl)
#set(_ri_inl_xsl ${ri_headers_SOURCE_DIR}/ri_inl.xsl)
#add_custom_command(OUTPUT ${_ri_inl}
#	COMMAND ${AQSIS_XSLTPROC_EXECUTABLE}
#		-o ${_ri_inl} --path ${ri_headers_SOURCE_DIR}
#		${_ri_inl_xsl} ${api_xml}
#	COMMENT "Generating ri.inl via XSL transformation..."
#	DEPENDS ${api_xml} ${_ri_inl_xsl}
#)
