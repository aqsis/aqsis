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
