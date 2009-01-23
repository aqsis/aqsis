PROJECT(RIBAPI)

xsl_transform(${CMAKE_CURRENT_BINARY_DIR}/ri.inl api.xml
	STYLESHEET apiheader.xsl
)
xsl_transform(${CMAKE_CURRENT_BINARY_DIR}/ri_cache.inl api.xml
	STYLESHEET apicache.xsl
)
xsl_transform(${CMAKE_CURRENT_BINARY_DIR}/ri_validate.inl api.xml
	STYLESHEET apivalidate.xsl
)
xsl_transform(${CMAKE_CURRENT_BINARY_DIR}/ri_debug.inl api.xml
	STYLESHEET apidebug.xsl
)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/ri.inl ${PROJECT_SOURCE_DIR}/ri.h 
	DESTINATION ${INCLUDEDIR})
