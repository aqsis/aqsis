SET(BINDIR "bin" 
	CACHE STRING "Install location for binary files. (relative to CMAKE_INSTALL_PREFIX)")
SET(LIBDIR "lib" 
	CACHE STRING "Install location for shared libraries (relative to CMAKE_INSTALL_PREFIX)")
SET(RENDERENGINEDIR "${LIBDIR}" 
	CACHE STRING "Install location for render dependent shared libraries (relative to CMAKE_INSTALL_PREFIX)")
SET(DISPLAYSDIR "${LIBDIR}/aqsis" 
	CACHE STRING "Install location for display libraries (relative to CMAKE_INSTALL_PREFIX)")
SET(PLUGINDIR "${LIBDIR}/aqsis/plugins" 
	CACHE STRING "Install location for plugin libraries (relative to CMAKE_INSTALL_PREFIX)")
SET(STATICLIBDIR "${LIBDIR}/aqsis" 
	CACHE STRING "Install location for static libraries (relative to CMAKE_INSTALL_PREFIX)")
SET(SHADERDIR "share/aqsis/shaders" 
	CACHE STRING "Install location for shaders (relative to CMAKE_INSTALL_PREFIX)")
SET(SYSCONFDIR "etc/aqsis" 
	CACHE STRING "Install location for system configuration files (relative to CMAKE_INSTALL_PREFIX)")
SET(INCLUDEDIR "include/aqsis" 
	CACHE STRING "Install location for aqsis header files (relative to CMAKE_INSTALL_PREFIX)")
SET(CONTENTDIR "content" 
	CACHE STRING "Install location for content (relative to CMAKE_INSTALL_PREFIX)")
SET(SCRIPTSDIR "scripts" 
	CACHE STRING "Install location for scripts (relative to CMAKE_INSTALL_PREFIX)")

IF(FIRST_CMAKE_RUN)
	# Override default compile flags the first time cmake is run.
	SET_IF_EMPTY(CMAKE_CXX_FLAGS "-Wall" CACHE STRING "Flags used by the compiler during all build types." FORCE)
	SET_IF_EMPTY(CMAKE_C_FLAGS "-Wall" CACHE STRING "Flags for C compiler." FORCE)
ENDIF(FIRST_CMAKE_RUN)

IF(AQSIS_USE_PLUGINS)
	SET(AQSISEXE_SYSTEM_LINKLIBS ${AQSIS_EXECUTABLE_LINKLIBS} dl)
	SET(TEQSER_SYSTEM_LINKLIBS ${AQSIS_EXECUTABLE_LINKLIBS} dl)
	SET(AQSL_SYSTEM_LINKLIBS ${AQSIS_EXECUTABLE_LINKLIBS} dl)
	SET(AQSLTELL_SYSTEM_LINKLIBS ${AQSIS_EXECUTABLE_LINKLIBS} dl)
	SET(MIQSER_SYSTEM_LINKLIBS ${AQSIS_EXECUTABLE_LINKLIBS} dl)
ENDIF(AQSIS_USE_PLUGINS)

#
# Packaging setup
#
SET(CPACK_GENERATOR "TGZ")
SET(CPACK_SOURCE_GENERATOR "TGZ")
SET(CPACK_STRIP_FILES ON)
