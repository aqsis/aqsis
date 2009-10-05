project(ri_headers)

set(ri_xml ${ri_headers_SOURCE_DIR}/ri.xml)

# Generate ri.inl from the XML specification.
set(ri_inl_name ${ri_headers_BINARY_DIR}/ri.inl)
xsl_transform(${ri_inl_name} ri.xml STYLESHEET ri_inl.xsl
	MAKE_ALL_TARGET ri_inl)

# Install all headers.
set(ri_install_hdrs
	ndspy.h
	#pointcloud.h
	ri.h
	ritypes.h
	rif.h
	shadeop.h
	slo.h
	slx.h
	${ri_inl_name}
)

install(FILES ${ri_install_hdrs} DESTINATION ${include_install_dir}/ri COMPONENT development)
