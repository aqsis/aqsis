SET(BINDIR "bin" 
	CACHE STRING "Install location for binary files. (relative to CMAKE_INSTALL_PREFIX)")
SET(PLUGINDIR "${BINDIR}" 
	CACHE STRING "Install location for plugins (relative to CMAKE_INSTALL_PREFIX)")
SET(LIBDIR "lib" 
	CACHE STRING "Install location for shared libraries (relative to CMAKE_INSTALL_PREFIX)")
SET(SHADERDIR "shaders" 
	CACHE STRING "Install location for shaders (relative to CMAKE_INSTALL_PREFIX)")
SET(SYSCONFDIR "${BINDIR}" 
	CACHE STRING "Install location for system configuration files (relative to CMAKE_INSTALL_PREFIX)")
SET(INCLUDEDIR "include"
	CACHE STRING "Install location for aqsis header files (relative to CMAKE_INSTALL_PREFIX)")
SET(EXAMPLESDIR "examples" 
	CACHE STRING "Install location for examples (relative to CMAKE_INSTALL_PREFIX)")
SET(DOCSDIR "doc"
	CACHE STRING "Install location for documentation (relative to CMAKE_INSTALL_PREFIX)")
SET(NEQSUSDIR "plugins" 
	CACHE STRING "Install location for neqsus (relative paths are relative to CMAKE_INSTALL_PREFIX)")
SET(SCRIPTSDIR "scripts" 
	CACHE STRING "Install location for scripts (relative to CMAKE_INSTALL_PREFIX)")

IF(AQSIS_USE_POINTCLOUD)
	SET(SHADERPATH "${SHADERPATH}%AQSISHOME%/${BINDIR}:")
ENDIF(AQSIS_USE_POINTCLOUD)
IF(AQSIS_ENABLE_AIRDBO)
	SET(PROCEDURALPATH "${PROCEDURALPATH}%AIRHOME%/procedurals:")
ENDIF(AQSIS_ENABLE_AIRDBO)
IF(AQSIS_ENABLE_MASSIVE)
	SET(PROCEDURALPATH "${PROCEDURALPATH}%MASSIVE_HOME%/bin:")
ENDIF(AQSIS_ENABLE_MASSIVE)
IF(AQSIS_ENABLE_SIMBIONT)
	SET(SHADERPATH "${SHADERPATH}%SIMBIONT_RM_COMPONENTS%/../SimbiontRM:%SIMBIONT_RM_COMPONENTS%/..:")
ENDIF(AQSIS_ENABLE_SIMBIONT)

#
# Fix for CMake "imlib set but there is no CMAKE_IMPORT_LIBRARY_SUFFIX" build-time error
#
SET(CMAKE_IMPORT_LIBRARY_SUFFIX ".lib")

# Add defines to
#   (a) Make sure we don't try to use the syslog stuff
#   (b) Make sure that the math constants from math.h are defined - that is, M_PI etc.
ADD_DEFINITIONS(-DNO_SYSLOG -D_USE_MATH_DEFINES)

