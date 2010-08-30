project(ri_headers)

set(ri_xml ${ri_headers_SOURCE_DIR}/ri.xml)

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
)

install(FILES ${ri_install_hdrs} DESTINATION ${include_install_dir}/ri COMPONENT development)
