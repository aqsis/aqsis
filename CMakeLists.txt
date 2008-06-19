PROJECT(aqsis_all)

# Path to aqsis-specific include directories for cmake helper scripts.
SET(CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake")

INCLUDE(macros)

INCLUDE(FirstCMakeRun)
INCLUDE(OutOfSourceBuild)

# Find path to precompiled libs on windows.
INCLUDE(FindWin32Libs)


#-------------------------------------------------------------------------------
# Build-time options which can be set from ccmake or the command line
#--------------------------------------------------------------------
OPTION(AQSIS_USE_TIMERS "Enable performance timers ON/OFF" ON)
OPTION(AQSIS_USE_PLUGINS "Enable plugins (required for plugin display drivers) ON/OFF" ON)
OPTION(AQSIS_USE_PDIFF "Build the external pdiff perceptual image difference utility" OFF)
OPTION(AQSIS_ENABLE_TESTING "Enable unit testing ON/OFF" OFF)


#-------------------------------------------------------------------------------
# Find necessary library dependencies.
#-------------------------------------
# TODO: Consider the use of FIND_PACKAGE here
INCLUDE(FindTIFF)
INCLUDE(FindBoost)
INCLUDE(FindZLIB)

# TODO: Should be optional.  Create USE_ variables for optional dependencies.
INCLUDE(FindFLTK)

IF(NOT AQSIS_FLTK_FOUND)
	MESSAGE(STATUS "Cannot find FLTK, display, eqsl and piqsl will not be built")
ENDIF(NOT AQSIS_FLTK_FOUND)
IF(NOT AQSIS_FLUID_FOUND)
	MESSAGE(STATUS "Cannot find Fluid, eqsl and piqsl will not be built")
ENDIF(NOT AQSIS_FLUID_FOUND)

INCLUDE(FindOpenEXR)
# libjpeg is no longer needed until aqsis again supports jpeg images.
#INCLUDE(FindJPEG)


#-------------------------------------------------------------------------------
# Find build tools
#-----------------
INCLUDE(FindXsltproc)
INCLUDE(FindFlexBison)
INCLUDE(FindSVNVersion)
# Windows-specific build tools:
INCLUDE(FindNSIS)
# Apple-specific build tools:
INCLUDE(FindOsacompile)
INCLUDE(FindXcodebuild)
INCLUDE(FindResource)


#-------------------------------------------------------------------------------
# Set variables which control the build
#--------------------------------------

# Set the locations into which libraries and executables will be placed
# Note: This is only relevant to the build, not the install.
IF(WIN32)
	SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
ELSE(WIN32)
	SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
ENDIF(WIN32)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# Set default build type the first time cmake is run, but only if it's empty
# since an alternative value might have been specified on the command line,
# and we want this to override the default given below.
#
# We need to jump through some hoops here with the SET_IF_EMPTY macro, since
# CMAKE_BUILD_TYPE is a system-defined variable which means we can't get to it
# with SET() before it's defined.
IF(FIRST_CMAKE_RUN)
	SET_IF_EMPTY(CMAKE_BUILD_TYPE "Release" CACHE STRING
		"Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
		FORCE
		)
ENDIF(FIRST_CMAKE_RUN)


#-------------------------------------------------------------------------------
# Define macros which enable compile-time features
#-------------------------------------------------

IF(WIN32)
	ADD_DEFINITIONS(-DNO_SYSLOG -D_USE_MATH_DEFINES)
ELSE(WIN32)
	SET(DEFAULT_RC_PATH "${CMAKE_INSTALL_PREFIX}/${SYSCONFDIR}")
	ADD_DEFINITIONS(-DDEFAULT_RC_PATH="${DEFAULT_RC_PATH}")
ENDIF(WIN32)

IF(AQSIS_USE_TIMERS)
	ADD_DEFINITIONS(-DUSE_TIMERS)
ENDIF(AQSIS_USE_TIMERS)

IF(AQSIS_USE_PLUGINS)
	ADD_DEFINITIONS(-DPLUGINS)
ENDIF(AQSIS_USE_PLUGINS)


#-------------------------------------------------------------------------------
# System-specific includes
#-------------------------

# Include setup for system-specific paths, packaging etc
IF(UNIX AND NOT APPLE)
	INCLUDE(${CMAKE_SOURCE_DIR}/cmake/platform/linux.cmake) 
ELSE(UNIX AND NOT APPLE)
	IF(WIN32)
		INCLUDE(${CMAKE_SOURCE_DIR}/cmake/platform/windows.cmake)
	ELSE(WIN32)
		IF(APPLE)
			INCLUDE(${CMAKE_SOURCE_DIR}/cmake/platform/macosx.cmake)
			INCLUDE(${CMAKE_SOURCE_DIR}/cmake/platform/macosx.cmake)
		ENDIF(APPLE)
	ENDIF(WIN32)
ENDIF(UNIX AND NOT APPLE)


#-------------------------------------------------------------------------------
# Create version.h
#-----------------
SET(MAJOR 1)
SET(MINOR 3)
SET(BUILD 0)
SVN_REPOSITORY_VERSION( REVISION ${PROJECT_SOURCE_DIR} )
CONFIGURE_FILE(version.h.in.cmake ${PROJECT_BINARY_DIR}/version.h)
# Make sure the rest of the build can find version.h
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})


#-------------------------------------------------------------------------------
# Add directories for all subprojects
#------------------------------------
ADD_SUBDIRECTORY(argparse)
ADD_SUBDIRECTORY(rib/api)
ADD_SUBDIRECTORY(rib/rib2)
ADD_SUBDIRECTORY(rib/rib2ri)
ADD_SUBDIRECTORY(renderer/ddmanager)
ADD_SUBDIRECTORY(renderer/raytrace)
ADD_SUBDIRECTORY(shadercompiler/shaderexecenv)
ADD_SUBDIRECTORY(aqsistypes)
# Get the value of AQSISTYPES_LIBRARY from the aqsistypes project, so that
# compiling shaders at build time can proceed (aqsl needs the library).
GET_DIRECTORY_PROPERTY(AQSISTYPES_LIBRARY DIRECTORY aqsistypes DEFINITION AQSISTYPES_LIBRARY)
ADD_SUBDIRECTORY(texturing/texlib)
ADD_SUBDIRECTORY(shadercompiler/shadervm)
ADD_SUBDIRECTORY(renderer/render)
ADD_SUBDIRECTORY(renderer/aqsis)
ADD_SUBDIRECTORY(shadercompiler/slparse)
ADD_SUBDIRECTORY(shadercompiler/codegenvm)
ADD_SUBDIRECTORY(shadercompiler/aqsl)
# Get the value of AQSL_EXECUTABLE from the shadercompiler/aqsl project, so
# that compiling shaders at build time can proceed.
GET_DIRECTORY_PROPERTY(AQSL_EXECUTABLE DIRECTORY shadercompiler/aqsl DEFINITION AQSL_EXECUTABLE)
ADD_SUBDIRECTORY(shadercompiler/slxargs)
ADD_SUBDIRECTORY(shadercompiler/aqsltell)
ADD_SUBDIRECTORY(rib/ri2rib)
ADD_SUBDIRECTORY(rib/miqser)
ADD_SUBDIRECTORY(texturing/teqser)
IF(AQSIS_FLTK_FOUND)
	ADD_SUBDIRECTORY(displays/display)
ENDIF(AQSIS_FLTK_FOUND)
ADD_SUBDIRECTORY(thirdparty/tinyxml)
ADD_SUBDIRECTORY(displays/d_exr)
ADD_SUBDIRECTORY(displays/d_xpm)
ADD_SUBDIRECTORY(displays/d_sdcBMP)
ADD_SUBDIRECTORY(shaders)
IF(AQSIS_FLTK_FOUND AND AQSIS_FLUID_FOUND AND AQSIS_BOOST_FOUND)
	ADD_SUBDIRECTORY(displays/piqsl)
	GET_DIRECTORY_PROPERTY(piqsl_DISPLAYLIB DIRECTORY displays/piqsl DEFINITION DISPLAYLIB)
ENDIF(AQSIS_FLTK_FOUND AND AQSIS_FLUID_FOUND AND AQSIS_BOOST_FOUND)
IF(AQSIS_FLTK_FOUND AND AQSIS_FLUID_FOUND)
	ADD_SUBDIRECTORY(tools/eqsl)
ENDIF(AQSIS_FLTK_FOUND AND AQSIS_FLUID_FOUND)
IF(AQSIS_USE_PDIFF)
	ADD_SUBDIRECTORY(thirdparty/pdiff)
ENDIF(AQSIS_USE_PDIFF)


#-------------------------------------------------------------------------------
# Build aqsisrc
#--------------
IF(AQSIS_FLTK_FOUND)
	GET_DIRECTORY_PROPERTY(display_DISPLAYLIB DIRECTORY displays/display DEFINITION DISPLAYLIB)
ENDIF(AQSIS_FLTK_FOUND)
GET_DIRECTORY_PROPERTY(d_exr_DISPLAYLIB DIRECTORY displays/d_exr DEFINITION DISPLAYLIB)
GET_DIRECTORY_PROPERTY(d_bmp_DISPLAYLIB DIRECTORY displays/d_sdcBMP DEFINITION DISPLAYLIB)
GET_DIRECTORY_PROPERTY(d_xpm_DISPLAYLIB DIRECTORY displays/d_xpm DEFINITION DISPLAYLIB)
SET(DEFAULT_SHADERPATH ${CMAKE_INSTALL_PREFIX}/${SHADERDIR})
SET(DEFAULT_ARCHIVEPATH ${CMAKE_INSTALL_PREFIX})
SET(DEFAULT_TEXTUREPATH ${CMAKE_INSTALL_PREFIX})
SET(DEFAULT_DISPLAYPATH ${CMAKE_INSTALL_PREFIX}/${DISPLAYSDIR})
SET(DEFAULT_PROCEDURALPATH ${CMAKE_INSTALL_PREFIX})
SET(DEFAULT_RESOURCEPATH ${CMAKE_INSTALL_PREFIX})
SET(SHADERPATH "")
SET(ARCHIVEPATH "")
SET(TEXTUREPATH "")
SET(DISPLAYPATH "")
SET(PROCEDURALPATH "")
SET(RESOURCEPATH "")
CONFIGURE_FILE(aqsisrc.in.cmake ${PROJECT_BINARY_DIR}/aqsisrc)
ADD_CUSTOM_TARGET(aqsisrc ALL echo
	DEPENDS  ${PROJECT_BINARY_DIR}/aqsisrc
	)
INSTALL(FILES ${PROJECT_BINARY_DIR}/aqsisrc DESTINATION ${SYSCONFDIR})


#-------------------------------------------------------------------------------
# Packaging
#----------

# Set some variables used inside the installer script templates.
#
# TODO: Rename these to use the AQSIS_ prefix
SET(PACKAGE_NAME "Aqsis Renderer")
SET(PACKAGE_VENDOR "Aqsis Team")
SET(PACKAGE_COPYRIGHT "Copyright (C) 2008, ${PACKAGE_VENDOR}.")
SET(PACKAGE_COPYRIGHT_OTHER "RenderMan(R) Interface Procedures and Protocol are Copyright 1988, 1989, Pixar All Rights Reserved.")


# Windows NSIS Packaging
IF(WIN32)
	ADD_SUBDIRECTORY(distribution/win/nsis)
ENDIF(WIN32)

# Linux SPEC Packaging
IF(UNIX AND NOT APPLE)
	ADD_SUBDIRECTORY(distribution/linux/rpm)
ENDIF(UNIX AND NOT APPLE)

# OSX Bundle Packaging
IF(APPLE)
	ADD_SUBDIRECTORY(distribution/macosx/bundle)
ENDIF(APPLE)


# Generic Packaging setup
INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PACKAGE_NAME}")
SET(CPACK_PACKAGE_VENDOR "${PACKAGE_VENDOR}")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
#SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${BUILD}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Aqsis-${MAJOR}.${MINOR}")
INCLUDE(CPack)
