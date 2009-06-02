SET(BUNLE_CONTENTS "Contents"
	CACHE STRING "Install location for bundle content. (relative to CMAKE_INSTALL_PREFIX)")
SET(BUNDLE_RESOURCES "${BUNLE_CONTENTS}/Resources"
	CACHE STRING "Install location for bundle resources. (relative to CMAKE_INSTALL_PREFIX)")
SET(BINDIR "${BUNDLE_RESOURCES}/MacOS" 
	CACHE STRING "Install location for binary files. (relative to CMAKE_INSTALL_PREFIX)")
SET(LIBDIR "${BUNDLE_RESOURCES}/lib" 
	CACHE STRING "Install location for shared libraries (relative to CMAKE_INSTALL_PREFIX)")
SET(PLUGINDIR "${LIBDIR}" 
	CACHE STRING "Install location for plugins (relative to CMAKE_INSTALL_PREFIX)")
SET(SHADERDIR "${BUNDLE_RESOURCES}/shaders" 
	CACHE STRING "Install location for shaders (relative to CMAKE_INSTALL_PREFIX)")
SET(SYSCONFDIR "etc/aqsis" 
	CACHE STRING "Install location for system configuration files (relative to CMAKE_INSTALL_PREFIX)")
SET(INCLUDEDIR "${BUNDLE_RESOURCES}/include"
	CACHE STRING "Install location for aqsis header files (relative to CMAKE_INSTALL_PREFIX)")
SET(EXAMPLESDIR "share/aqsis/examples" 
	CACHE STRING "Install location for examples (relative to CMAKE_INSTALL_PREFIX)")
SET(SCRIPTSDIR "share/aqsis/scripts" 
	CACHE STRING "Install location for scripts (relative to CMAKE_INSTALL_PREFIX)")

IF(AQSIS_USE_POINTCLOUD)
	SET(SHADERPATH "${SHADERPATH}${CMAKE_INSTALL_PREFIX}/${LIBDIR}:")
ENDIF(AQSIS_USE_POINTCLOUD)
IF(AQSIS_ENABLE_MASSIVE)
	SET(PROCEDURALPATH "%MASSIVE_HOME%/bin:")
ENDIF(AQSIS_ENABLE_MASSIVE)

# Ensure that the Carbons libraries are found and used appropriately
INCLUDE_DIRECTORIES ( /Developer/Headers/FlatCarbon )
FIND_LIBRARY(CARBON_LIBRARY Carbon)
#FIND_LIBRARY(QUICKTIME_LIBRARY QuickTime )
FIND_LIBRARY(APP_SERVICES_LIBRARY ApplicationServices )
MARK_AS_ADVANCED (CARBON_LIBRARY)
#MARK_AS_ADVANCED (QUICKTIME_LIBRARY)
MARK_AS_ADVANCED (APP_SERVICES_LIBRARY)

FILE(RELATIVE_PATH LIB_REL_TO_BIN ${CMAKE_INSTALL_PREFIX}/${BINDIR} ${CMAKE_INSTALL_PREFIX}/${LIBDIR})
SET(CMAKE_INSTALL_NAME_DIR "@executable_path/${LIB_REL_TO_BIN}")

