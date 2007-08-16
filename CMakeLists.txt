PROJECT(aqsis_all)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

INCLUDE(macros)

INCLUDE(OutOfSourceBuild)

INCLUDE(FindWin32Libs)
INCLUDE(FindTIFF)
INCLUDE(FindBoost)
INCLUDE(FindZLIB)
INCLUDE(FindFLTK)
INCLUDE(FindOpenEXR)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# TODO: find these properly
SET(FLEX_EXECUTABLE flex)
SET(BISON_EXECUTABLE bison)

SET(MAJOR 1)
SET(MINOR 3)
SET(BUILD 0)
CONFIGURE_FILE(version.h.in.cmake ${PROJECT_BINARY_DIR}/version.h)

INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

IF(WIN32)
	SET(BINDIR "bin" 
		CACHE STRING "Install location for binary files. (relative to CMAKE_INSTALL_PREFIX)")
	SET(RENDERENGINEDIR "${BINDIR}" 
		CACHE STRING "Install location for render dependent shared libraries (relative to CMAKE_INSTALL_PREFIX)")
	SET(DISPLAYSDIR "${BINDIR}" 
		CACHE STRING "Install location for display libraries (relative to CMAKE_INSTALL_PREFIX)")
	SET(PLUGINDIR "${BINDIR}" 
		CACHE STRING "Install location for plugin libraries (relative to CMAKE_INSTALL_PREFIX)")
	SET(LIBDIR "lib" 
		CACHE STRING "Install location for shared libraries (relative to CMAKE_INSTALL_PREFIX)")
	SET(STATICLIBDIR "${LIBDIR}" 
		CACHE STRING "Install location for static libraries (relative to CMAKE_INSTALL_PREFIX)")
	SET(SHADERDIR "shaders" 
		CACHE STRING "Install location for shaders (relative to CMAKE_INSTALL_PREFIX)")
	SET(SYSCONFDIR "${BINDIR}" 
		CACHE STRING "Install location for system configuration files (relative to CMAKE_INSTALL_PREFIX)")
	SET(INCLUDEDIR "include/aqsis" 
		CACHE STRING "Install location for aqsis header files (relative to CMAKE_INSTALL_PREFIX)")
	SET(CONTENTDIR "content" 
		CACHE STRING "Install location for content (relative to CMAKE_INSTALL_PREFIX)")
	SET(SCRIPTSDIR "scripts" 
		CACHE STRING "Install location for scripts (relative to CMAKE_INSTALL_PREFIX)")
ELSE(WIN32)
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
ENDIF(WIN32)

SET(DEFAULT_PLUGIN_PATH "${CMAKE_INSTALL_PREFIX}/${PLUGINDIR}")
SET(DEFAULT_RC_PATH "${CMAKE_INSTALL_PREFIX}/${SYSCONFDIR}")

IF(WIN32)
	ADD_DEFINITIONS(-DNO_SYSLOG)
ELSE(WIN32)
	ADD_DEFINITIONS(-DDEFAULT_PLUGIN_PATH="\\"${DEFAULT_PLUGIN_PATH}\\"")
	ADD_DEFINITIONS(-DDEFAULT_RC_PATH="\\"${DEFAULT_RC_PATH}\\"")
ENDIF(WIN32)

ADD_SUBDIRECTORY(argparse)
ADD_SUBDIRECTORY(rib/api)
ADD_SUBDIRECTORY(rib/rib2)
ADD_SUBDIRECTORY(rib/rib2ri)
ADD_SUBDIRECTORY(renderer/ddmanager)
ADD_SUBDIRECTORY(renderer/raytrace)
ADD_SUBDIRECTORY(shadercompiler/shaderexecenv)
ADD_SUBDIRECTORY(aqsistypes)
ADD_SUBDIRECTORY(shadercompiler/shadervm)
ADD_SUBDIRECTORY(renderer/render)
ADD_SUBDIRECTORY(renderer/aqsis)
ADD_SUBDIRECTORY(shadercompiler/slpp)
ADD_SUBDIRECTORY(shadercompiler/slparse)
ADD_SUBDIRECTORY(shadercompiler/codegenvm)
ADD_SUBDIRECTORY(shadercompiler/aqsl)
ADD_SUBDIRECTORY(shadercompiler/slxargs)
ADD_SUBDIRECTORY(shadercompiler/aqsltell)
ADD_SUBDIRECTORY(rib/ri2rib)
ADD_SUBDIRECTORY(texturing/teqser)
ADD_SUBDIRECTORY(displays/display)
ADD_SUBDIRECTORY(thirdparty/tinyxml)
ADD_SUBDIRECTORY(displays/piqsl)
ADD_SUBDIRECTORY(displays/d_exr)
ADD_SUBDIRECTORY(displays/d_sdcBMP)
# Get the value of AQSL_EXECUTABLE from the shadercompiler/aqsl project, so that downstream projects
# can use it.
GET_DIRECTORY_PROPERTY(AQSL_EXECUTABLE DIRECTORY shadercompiler/aqsl DEFINITION AQSL_EXECUTABLE)
ADD_SUBDIRECTORY(shaders)

#
# Build aqsisrc
#
GET_DIRECTORY_PROPERTY(display_DISPLAYLIB DIRECTORY displays/display DEFINITION DISPLAYLIB)
GET_DIRECTORY_PROPERTY(piqsl_DISPLAYLIB DIRECTORY displays/piqsl DEFINITION DISPLAYLIB)
GET_DIRECTORY_PROPERTY(d_exr_DISPLAYLIB DIRECTORY displays/d_exr DEFINITION DISPLAYLIB)
GET_DIRECTORY_PROPERTY(d_bmp_DISPLAYLIB DIRECTORY displays/d_sdcBMP DEFINITION DISPLAYLIB)
CONFIGURE_FILE(aqsisrc.in.cmake ${PROJECT_BINARY_DIR}/aqsisrc)
ADD_CUSTOM_TARGET(aqsisrc ALL echo
	DEPENDS  ${PROJECT_BINARY_DIR}/aqsisrc
	)
INSTALL(FILES ${PROJECT_BINARY_DIR}/aqsisrc DESTINATION ${SYSCONFDIR})

#
# Packaging setup
#
INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Aqsis")
SET(CPACK_PACKAGE_VENDOR "The Aqsis Team")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${BUILD}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Aqsis-${MAJOR}.${MINOR}")
IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  #SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\aqsis_install.exe")
  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Aqsis")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.aqsis.org")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.aqsis.org")
  SET(CPACK_NSIS_CONTACT "aqsis@aqsis.org")
  SET(CPACK_NSIS_MODIFY_PATH ON)
ELSE(WIN32 AND NOT UNIX)
  SET(CPACK_STRIP_FILES ON)
ENDIF(WIN32 AND NOT UNIX)
INCLUDE(CPack)
