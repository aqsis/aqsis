PROJECT(aqsis_all)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

INCLUDE(FindTIFF)
INCLUDE(FindBoost)
INCLUDE(FindZLIB)
INCLUDE(FindFLTK)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# TODO: This should probably be in a separate macros file.
MACRO(FILTER_OUT FILTERS INPUTS OUTPUT)
   # Mimicks Gnu Make's $(filter-out) which removes elements 
   # from a list that match the pattern.
   # Arguments:
   #  FILTERS - list of patterns that need to be removed
   #  INPUTS  - list of inputs that will be worked on
   #  OUTPUT  - the filtered list to be returned
   # 
   # Example: 
   #  SET(MYLIST this that and the other)
   #  SET(FILTS this that)
   #
   #  FILTER_OUT("${FILTS}" "${MYLIST}" OUT)
   #  MESSAGE("OUTPUT = ${OUT}")
   #
   # The output - 
   #   OUTPUT = and;the;other
   #
   SET(FOUT "")
   FOREACH(INP ${INPUTS})
	   SET(FILTERED 0)
	   FOREACH(FILT ${FILTERS})
		   IF(${FILTERED} EQUAL 0)
			   IF("${FILT}" STREQUAL "${INP}")
				   SET(FILTERED 1)
			   ENDIF("${FILT}" STREQUAL "${INP}")
		   ENDIF(${FILTERED} EQUAL 0)
	   ENDFOREACH(FILT ${FILTERS})
	   IF(${FILTERED} EQUAL 0)
		   SET(FOUT ${FOUT} ${INP})
	   ENDIF(${FILTERED} EQUAL 0)
   ENDFOREACH(INP ${INPUTS})
   SET(${OUTPUT} ${FOUT})
ENDMACRO(FILTER_OUT FILTERS INPUTS OUTPUT)

# TODO: find these properly
SET(FLEX_EXECUTABLE flex)
SET(BISON_EXECUTABLE bison)

IF(WIN32)
	ADD_DEFINITIONS(-DNO_SYSLOG)
ELSE(WIN32)
	ADD_DEFINITIONS(-DDEFAULT_PLUGIN_PATH="\\"/usr/local/aqsis/lib/aqsis/plugins\\"")
	ADD_DEFINITIONS(-DDEFAULT_RC_PATH="\\"/usr/local/aqsis/lib/aqsis/plugins\\"")
ENDIF(WIN32)

SET(MAJOR 1)
SET(MINOR 3)
SET(BUILD 0)
CONFIGURE_FILE(version.h.in.cmake ${PROJECT_BINARY_DIR}/version.h)

INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

SET(BINDIR "bin" CACHE STRING "Install location for binary files. (relative to CMAKE_INSTALL_PREFIX)")
SET(RENDERENGINEDIR "${BINDIR}" CACHE STRING "Install location for render dependent shared libraries (relative to CMAKE_INSTALL_PREFIX)")
SET(DISPLAYSDIR "${BINDIR}" CACHE STRING "Install location for display libraries (relative to CMAKE_INSTALL_PREFIX)")
SET(PLUGINDIR "${BINDIR}" CACHE STRING "Install location for plugin libraries (relative to CMAKE_INSTALL_PREFIX)")
SET(LIBDIR "lib" CACHE STRING "Install location for shared libraries (relative to CMAKE_INSTALL_PREFIX)")
SET(STATICLIBDIR "${LIBDIR}" CACHE STRING "Install location for static libraries (relative to CMAKE_INSTALL_PREFIX)")
SET(SHADERDIR "shaders" CACHE STRING "Install location for shaders (relative to CMAKE_INSTALL_PREFIX)")
SET(SYSCONFDIR "${BINDIR}" CACHE STRING "Install location for system configuration files (relative to CMAKE_INSTALL_PREFIX)")
SET(INCLUDEDIR "include/aqsis" CACHE STRING "Install location for aqsis header files (relative to CMAKE_INSTALL_PREFIX)")
SET(CONTENTDIR "content" CACHE STRING "Install location for content (relative to CMAKE_INSTALL_PREFIX)")
SET(SCRIPTSDIR "scripts" CACHE STRING "Install location for scripts (relative to CMAKE_INSTALL_PREFIX)")

SUBDIRS(argparse)
SUBDIRS(rib/api)
SUBDIRS(rib/rib2)
SUBDIRS(rib/rib2ri)
SUBDIRS(renderer/ddmanager)
SUBDIRS(renderer/raytrace)
SUBDIRS(shadercompiler/shaderexecenv)
SUBDIRS(aqsistypes)
SUBDIRS(shadercompiler/shadervm)
SUBDIRS(renderer/render)
SUBDIRS(renderer/aqsis)
SUBDIRS(shadercompiler/slpp)
SUBDIRS(shadercompiler/slparse)
SUBDIRS(shadercompiler/codegenvm)
SUBDIRS(shadercompiler/aqsl)
SUBDIRS(shadercompiler/slxargs)
SUBDIRS(shadercompiler/aqsltell)
SUBDIRS(rib/ri2rib)
SUBDIRS(texturing/teqser)
SUBDIRS(displays/display)
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
