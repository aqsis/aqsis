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

IF(AQSIS_ENABLE_AIRDBO)
	SET(PROCEDURALPATH "${PROCEDURALPATH}%AIRHOME%\\procedurals:")
ENDIF(AQSIS_ENABLE_AIRDBO)
IF(AQSIS_ENABLE_MASSIVE)
	SET(PROCEDURALPATH "${PROCEDURALPATH}%MASSIVE_HOME%\\bin:")
ENDIF(AQSIS_ENABLE_MASSIVE)
IF(AQSIS_ENABLE_SIMBIONT)
	SET(SHADERPATH "${SHADERPATH}%SIMBIONT_RM_COMPONENTS%\\..\\SimbiontRM:%SIMBIONT_RM_COMPONENTS%\\..:")
ENDIF(AQSIS_ENABLE_SIMBIONT)

# Add defines to
#   (a) Make sure we don't try to use the syslog stuff
#   (b) Make sure that the math constants from math.h are defined - that is, M_PI etc.
ADD_DEFINITIONS(-DNO_SYSLOG -D_USE_MATH_DEFINES)

SET(AQSISTYPES_SYSTEM_LINKLIBS ${AQSISTYPES_SYSTEM_LINKLIBS} ws2_32)

#
# Create resource files for use during linking
#
CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/distribution/win/info.rc.in.cmake" "${PROJECT_BINARY_DIR}/info.rc")
CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/distribution/win/icon.rc.in.cmake" "${PROJECT_BINARY_DIR}/icon.rc")
SET(INFORES_SRCS "${PROJECT_BINARY_DIR}/info.rc")
SET(ICONRES_SRCS "${PROJECT_BINARY_DIR}/icon.rc")

# There is a bug in NSI that does not handle full unix paths properly. Make
# sure there is at least one set of four (4) backlasshes.
#SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
#
# Packaging setup
#
SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\aqsis_install.exe")
SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Aqsis")
SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.aqsis.org\\\\xoops\\\\modules\\\\newbb")
SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.aqsis.org")
SET(CPACK_NSIS_CONTACT "packages@aqsis.org")
SET(CPACK_NSIS_MODIFY_PATH ON)
SET(CPACK_SOURCE_GENERATOR "ZIP")
