PROJECT(RIBAPI)

xsl_transform(${CMAKE_CURRENT_BINARY_DIR}/ri.inl api.xml
	STYLESHEET apiheader.xsl
	MAKE_ALL_TARGET apiheader
)
xsl_transform(${CMAKE_CURRENT_BINARY_DIR}/ri_cache.inl api.xml
	STYLESHEET apicache.xsl
	MAKE_ALL_TARGET apicache
)
xsl_transform(${CMAKE_CURRENT_BINARY_DIR}/ri_validate.inl api.xml
	STYLESHEET apivalidate.xsl
	MAKE_ALL_TARGET apivalidate
)
xsl_transform(${CMAKE_CURRENT_BINARY_DIR}/ri_debug.inl api.xml
	STYLESHEET apidebug.xsl
	MAKE_ALL_TARGET apidebug
)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/ri.inl ${PROJECT_SOURCE_DIR}/ri.h 
	DESTINATION ${INCLUDEDIR})
