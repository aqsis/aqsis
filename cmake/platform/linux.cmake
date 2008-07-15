SET(BINDIR "bin" 
	CACHE STRING "Install location for binary files. (relative paths are relative to CMAKE_INSTALL_PREFIX)")
SET(LIBDIR "lib" 
	CACHE STRING "Install location for shared libraries (relative paths are relative to CMAKE_INSTALL_PREFIX)")
SET(DISPLAYSDIR "${LIBDIR}/aqsis" 
	CACHE STRING "Install location for display libraries (relative paths are relative to CMAKE_INSTALL_PREFIX)")
SET(SHADERDIR "share/aqsis/shaders" 
	CACHE STRING "Install location for shaders (relative paths are relative to CMAKE_INSTALL_PREFIX)")
SET(SYSCONFDIR "etc/aqsis" 
	CACHE STRING "Install location for system configuration files (relative paths are relative to CMAKE_INSTALL_PREFIX)")
SET(INCLUDEDIR "include/aqsis" 
	CACHE STRING "Install location for aqsis header files (relative paths are relative to CMAKE_INSTALL_PREFIX)")
SET(CONTENTDIR_NAME "content"
	CACHE STRING "Name of content directory")
SET(CONTENTDIR "share/aqsis/${CONTENTDIR_NAME}" 
	CACHE STRING "Install location for content (relative paths are relative to CMAKE_INSTALL_PREFIX)")
SET(SCRIPTSDIR_NAME "scripts"
	CACHE STRING "Name of scripts directory")
SET(SCRIPTSDIR "share/aqsis/${SCRIPTSDIR_NAME}" 
	CACHE STRING "Install location for scripts (relative paths are relative to CMAKE_INSTALL_PREFIX)")
SET(DESKTOPDIR_NAME "desktop"
	CACHE STRING "Name of desktop integration directory")
SET(DESKTOPDIR "share/aqsis/${DESKTOPDIR_NAME}" 
	CACHE STRING "Install location for desktop integration (relative paths are relative to CMAKE_INSTALL_PREFIX)")

IF(AQSIS_ENABLE_AIRDBO)
	SET(PROCEDURALPATH "${PROCEDURALPATH}%AIRHOME%/procedurals:")
ENDIF(AQSIS_ENABLE_AIRDBO)
IF(AQSIS_ENABLE_MASSIVE)
	SET(PROCEDURALPATH "${PROCEDURALPATH}%MASSIVE_HOME%/bin:")
ENDIF(AQSIS_ENABLE_MASSIVE)

IF(FIRST_CMAKE_RUN)
	# Override default compile flags the first time cmake is run.
	SET_IF_EMPTY(CMAKE_CXX_FLAGS "-Wall" CACHE STRING "Flags used by the compiler during all build types." FORCE)
	SET_IF_EMPTY(CMAKE_C_FLAGS "-Wall" CACHE STRING "Flags for C compiler." FORCE)
ENDIF(FIRST_CMAKE_RUN)

IF(AQSIS_USE_PLUGINS)
	SET(AQSISTYPES_SYSTEM_LINKLIBS ${AQSISTYPES_SYSTEM_LINKLIBS} dl)
ENDIF(AQSIS_USE_PLUGINS)

# Determine the default location of aqsisrc.  This depends on whether
# SYSCONFDIR is an absolute or relative path.  Relative paths are relative to
# CMAKE_INSTALL_PREFIX.
STRING(SUBSTRING ${SYSCONFDIR} 0 1 SYSCONFDIR_PREFIX)
STRING(COMPARE EQUAL ${SYSCONFDIR_PREFIX} "/" SYSCONFDIR_IS_ABSOLUTE_PATH)
IF(SYSCONFDIR_IS_ABSOLUTE_PATH)
	SET(DEFAULT_RC_PATH "${SYSCONFDIR}")
ELSE(SYSCONFDIR_IS_ABSOLUTE_PATH)
	SET(DEFAULT_RC_PATH "${CMAKE_INSTALL_PREFIX}/${SYSCONFDIR}")
ENDIF(SYSCONFDIR_IS_ABSOLUTE_PATH)
# Add a define for the default location of aqsisrc.
ADD_DEFINITIONS(-DDEFAULT_RC_PATH="${DEFAULT_RC_PATH}")

#
# Packaging setup
#
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${AQSIS_PROJECT_NAME_BASIC}-${MAJOR}.${MINOR}.${BUILD}")
SET(CPACK_GENERATOR "TGZ")
SET(CPACK_SOURCE_GENERATOR "TGZ")
