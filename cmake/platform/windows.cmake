SET(BINDIR "bin" 
	CACHE STRING "Install location for binary files. (relative to CMAKE_INSTALL_PREFIX)")
SET(DISPLAYSDIR "${BINDIR}" 
	CACHE STRING "Install location for display libraries (relative to CMAKE_INSTALL_PREFIX)")
SET(LIBDIR "lib" 
	CACHE STRING "Install location for shared libraries (relative to CMAKE_INSTALL_PREFIX)")
SET(SHADERDIR "shaders" 
	CACHE STRING "Install location for shaders (relative to CMAKE_INSTALL_PREFIX)")
SET(SYSCONFDIR "${BINDIR}" 
	CACHE STRING "Install location for system configuration files (relative to CMAKE_INSTALL_PREFIX)")
SET(INCLUDEDIR "include/aqsis" 
	CACHE STRING "Install location for aqsis header files (relative to CMAKE_INSTALL_PREFIX)")
SET(CONTENTDIR_NAME "content"
	CACHE STRING "Name of content directory")
SET(CONTENTDIR "${CONTENTDIR_NAME}" 
	CACHE STRING "Install location for content (relative to CMAKE_INSTALL_PREFIX)")
SET(SCRIPTSDIR_NAME "scripts"
	CACHE STRING "Name of scripts directory")
SET(SCRIPTSDIR "${SCRIPTSDIR_NAME}" 
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

# Add defines to
#   (a) Make sure we don't try to use the syslog stuff
#   (b) Make sure that the math constants from math.h are defined - that is, M_PI etc.
ADD_DEFINITIONS(-DNO_SYSLOG -D_USE_MATH_DEFINES)

#
# Fix for CMake "imlib set but there is no CMAKE_IMPORT_LIBRARY_SUFFIX" build-time error
#
SET(CMAKE_IMPORT_LIBRARY_SUFFIX ".lib")


#
# Create resource files for use during linking
#
CONFIGURE_FILE("$ENV{CMAKE_SOURCE_DIR}/distribution/win/info.rc.in.cmake" "${CMAKE_BINARY_DIR}/info.rc")
CONFIGURE_FILE("$ENV{CMAKE_SOURCE_DIR}/distribution/win/icon.rc.in.cmake" "${CMAKE_BINARY_DIR}/icon.rc")
SET(INFORES_SRCS "${CMAKE_BINARY_DIR}/info.rc")
SET(ICONRES_SRCS "${CMAKE_BINARY_DIR}/icon.rc")

