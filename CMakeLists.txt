PROJECT(aqsis_all)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

INCLUDE(macros)

INCLUDE(OutOfSourceBuild)

INCLUDE(FindWin32Libs)
INCLUDE(FindTIFF)
INCLUDE(FindJPEG)
INCLUDE(FindBoost)
INCLUDE(FindZLIB)
INCLUDE(FindFLTK)
INCLUDE(FindOpenEXR)
INCLUDE(FindXsltproc)
INCLUDE(FindFlexBison)

ENABLE_TESTING()

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

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
ENDIF(WIN32)

OPTION(AQSIS_USE_TIMERS "Enable performance timers ON/OFF" ON)
IF(AQSIS_USE_TIMERS)
	ADD_DEFINITIONS(-DUSE_TIMERS)
ENDIF(AQSIS_USE_TIMERS)

OPTION(AQSIS_USE_PLUGINS "Enable plugins (required for plugin display drivers) ON/OFF" ON)
IF(AQSIS_USE_PLUGINS)
	ADD_DEFINITIONS(-DPLUGINS)
ENDIF(AQSIS_USE_PLUGINS)

OPTION(AQSIS_ENABLE_TESTING "Enable unit testing ON/OFF" OFF)

IF(UNIX AND NOT APPLE)
	INCLUDE(${CMAKE_SOURCE_DIR}/cmake/platform/linux.cmake) 
ELSE(UNIX AND NOT APPLE)
	IF(WIN32)
	ELSE(WIN32)
		IF(APPLE)
			INCLUDE(${CMAKE_SOURCE_DIR}/cmake/platform/macosx.cmake)
			INCLUDE(${CMAKE_SOURCE_DIR}/cmake/platform/macosx.cmake)
		ENDIF(APPLE)
	ENDIF(WIN32)
ENDIF(UNIX AND NOT APPLE)

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
# Get the value of AQSISTYPES_LIBRARY from the aqsistypes project, so that downstream projects
# can use it.
GET_DIRECTORY_PROPERTY(AQSISTYPES_LIBRARY DIRECTORY aqsistypes DEFINITION AQSISTYPES_LIBRARY)
ADD_SUBDIRECTORY(shadercompiler/shadervm)
ADD_SUBDIRECTORY(renderer/render)
ADD_SUBDIRECTORY(renderer/aqsis)
ADD_SUBDIRECTORY(shadercompiler/slpp)
ADD_SUBDIRECTORY(shadercompiler/slparse)
ADD_SUBDIRECTORY(shadercompiler/codegenvm)
ADD_SUBDIRECTORY(shadercompiler/aqsl)
# Get the value of AQSL_EXECUTABLE from the shadercompiler/aqsl project, so that downstream projects
# can use it.
GET_DIRECTORY_PROPERTY(AQSL_EXECUTABLE DIRECTORY shadercompiler/aqsl DEFINITION AQSL_EXECUTABLE)
ADD_SUBDIRECTORY(shadercompiler/slxargs)
ADD_SUBDIRECTORY(shadercompiler/aqsltell)
ADD_SUBDIRECTORY(rib/ri2rib)
ADD_SUBDIRECTORY(texturing/teqser)
ADD_SUBDIRECTORY(displays/display)
ADD_SUBDIRECTORY(thirdparty/tinyxml)
ADD_SUBDIRECTORY(displays/d_exr)
ADD_SUBDIRECTORY(displays/d_sdcBMP)
ADD_SUBDIRECTORY(texturing/plugins/common)
ADD_SUBDIRECTORY(texturing/plugins/ppm2tif)
ADD_SUBDIRECTORY(texturing/plugins/bake2tif)
ADD_SUBDIRECTORY(texturing/plugins/gif2tif)
ADD_SUBDIRECTORY(texturing/plugins/pcx2tif)
ADD_SUBDIRECTORY(texturing/plugins/jpg2tif)
ADD_SUBDIRECTORY(texturing/plugins/bmp2tif)
ADD_SUBDIRECTORY(texturing/plugins/exr2tif)
ADD_SUBDIRECTORY(texturing/plugins/png2tif)
ADD_SUBDIRECTORY(texturing/plugins/tga2tif)
ADD_SUBDIRECTORY(shaders)

IF(AQSIS_FLTK_FOUND AND AQSIS_BOOST_FOUND)
	ADD_SUBDIRECTORY(displays/piqsl)
	GET_DIRECTORY_PROPERTY(piqsl_DISPLAYLIB DIRECTORY displays/piqsl DEFINITION DISPLAYLIB)
ENDIF(AQSIS_FLTK_FOUND AND AQSIS_BOOST_FOUND)
#
# Build aqsisrc
#
GET_DIRECTORY_PROPERTY(display_DISPLAYLIB DIRECTORY displays/display DEFINITION DISPLAYLIB)
GET_DIRECTORY_PROPERTY(d_exr_DISPLAYLIB DIRECTORY displays/d_exr DEFINITION DISPLAYLIB)
GET_DIRECTORY_PROPERTY(d_bmp_DISPLAYLIB DIRECTORY displays/d_sdcBMP DEFINITION DISPLAYLIB)
SET(SHADERPATH ${CMAKE_INSTALL_PREFIX}/${SHADERDIR})
SET(DISPLAYPATH ${CMAKE_INSTALL_PREFIX}/${DISPLAYSDIR})
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
#SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
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
#ELSE(WIN32 AND NOT UNIX)
  SET(CPACK_STRIP_FILES ON)
ENDIF(WIN32 AND NOT UNIX)
INCLUDE(CPack)