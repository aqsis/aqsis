SET(BINDIR "bin" 
	CACHE STRING "Install location for binary files. (relative paths are relative to CMAKE_INSTALL_PREFIX)")
SET(LIBDIR "lib" 
	CACHE STRING "Install location for shared libraries (relative paths are relative to CMAKE_INSTALL_PREFIX)")
SET(PLUGINDIR "${LIBDIR}/aqsis" 
	CACHE STRING "Install location for plugins (relative paths are relative to CMAKE_INSTALL_PREFIX)")
SET(SHADERDIR "share/aqsis/shaders" 
	CACHE STRING "Install location for shaders (relative paths are relative to CMAKE_INSTALL_PREFIX)")
SET(SYSCONFDIR "etc/aqsis" 
	CACHE STRING "Install location for system configuration files (relative paths are relative to CMAKE_INSTALL_PREFIX)")
SET(INCLUDEDIR "include"
	CACHE STRING "Install location for aqsis header files (relative paths are relative to CMAKE_INSTALL_PREFIX)")
SET(EXAMPLESDIR "share/aqsis/examples" 
	CACHE STRING "Install location for examples (relative paths are relative to CMAKE_INSTALL_PREFIX)")
SET(DOCSDIR "share/aqsis/doc"
	CACHE STRING "Install location for documentation (relative paths are relative to CMAKE_INSTALL_PREFIX)")
SET(NEQSUSDIR "share/aqsis/plugins" 
	CACHE STRING "Install location for neqsus (relative paths are relative to CMAKE_INSTALL_PREFIX)")
SET(SCRIPTSDIR "share/aqsis/scripts" 
	CACHE STRING "Install location for scripts (relative paths are relative to CMAKE_INSTALL_PREFIX)")

IF(AQSIS_USE_POINTCLOUD)
	SET(SHADERPATH "${SHADERPATH}${CMAKE_INSTALL_PREFIX}/${LIBDIR}:")
ENDIF(AQSIS_USE_POINTCLOUD)
IF(AQSIS_ENABLE_AIRDBO)
	SET(PROCEDURALPATH "${PROCEDURALPATH}%AIRHOME%/procedurals:")
ENDIF(AQSIS_ENABLE_AIRDBO)
IF(AQSIS_ENABLE_MASSIVE)
	SET(PROCEDURALPATH "${PROCEDURALPATH}%MASSIVE_HOME%/bin:")
ENDIF(AQSIS_ENABLE_MASSIVE)

ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/distribution/linux)

IF(FIRST_CMAKE_RUN)
	# Override default compile flags the first time cmake is run.
	SET_IF_EMPTY(CMAKE_CXX_FLAGS "-Wall" CACHE STRING "Flags used by the compiler during all build types." FORCE)
	SET_IF_EMPTY(CMAKE_C_FLAGS "-Wall" CACHE STRING "Flags for C compiler." FORCE)
ENDIF(FIRST_CMAKE_RUN)

# Determine the default location of aqsisrc.  This depends on whether
# SYSCONFDIR is an absolute or relative path.  Relative paths are relative to
# CMAKE_INSTALL_PREFIX.
SET_WITH_PATH_PREFIX(DEFAULT_RC_PATH "${SYSCONFDIR}" "${CMAKE_INSTALL_PREFIX}")
