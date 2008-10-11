PROJECT(aqsis_all)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6)

# Set some variables used within various resources, including installer scripts.
SET(AQSIS_PROJECT_NAME "Aqsis Renderer")
SET(AQSIS_PROJECT_NAME_SHORT "Aqsis")
SET(AQSIS_PROJECT_NAME_BASIC "aqsis")
SET(AQSIS_PROJECT_VENDOR "Aqsis Team")
SET(AQSIS_PROJECT_COPYRIGHT "Copyright (C) 2008, ${AQSIS_PROJECT_VENDOR}.")
SET(AQSIS_PROJECT_COPYRIGHT_OTHER "RenderMan(R) Interface Procedures and Protocol are Copyright 1988, 1989, Pixar All Rights Reserved.")

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
OPTION(AQSIS_USE_PDIFF "Build the external pdiff perceptual image difference utility" OFF)
MARK_AS_ADVANCED(AQSIS_USE_PDIFF)
OPTION(AQSIS_USE_FLTK "Build the aqsis GUI components which rely on the fltk library" ON)
OPTION(AQSIS_USE_OPENEXR "Build aqsis with support for the OpenEXR image format" ON)
OPTION(AQSIS_USE_EXTERNAL_TINYXML "Try to find and use an external tinyxml library" OFF)
MARK_AS_ADVANCED(AQSIS_USE_EXTERNAL_TINYXML)

OPTION(AQSIS_ENABLE_TESTING "Enable unit testing ON/OFF" OFF)
OPTION(AQSIS_ENABLE_MPDUMP "Enable micropolygon dumping code ON/OFF" OFF)
MARK_AS_ADVANCED(AQSIS_ENABLE_MPDUMP)
OPTION(AQSIS_ENABLE_AIRDBO "Enable AIR DBO support ON/OFF" ON)
MARK_AS_ADVANCED(AQSIS_ENABLE_AIRDBO)
OPTION(AQSIS_ENABLE_MASSIVE "Enable Massive support ON/OFF" ON)
MARK_AS_ADVANCED(AQSIS_ENABLE_MASSIVE)
OPTION(AQSIS_ENABLE_SIMBIONT "Enable Simbiont(RM) support ON/OFF" ON)
MARK_AS_ADVANCED(AQSIS_ENABLE_SIMBIONT)

OPTION(AQSIS_USE_RPATH "Enable runtime path for installed libs" ON)
MARK_AS_ADVANCED(AQSIS_USE_RPATH)

SET(AQSIS_MAIN_CONFIG_NAME "aqsisrc" CACHE STRING "Name of the main aqsis configuration file")
MARK_AS_ADVANCED(AQSIS_MAIN_CONFIG_NAME)


#-------------------------------------------------------------------------------
# Find necessary library dependencies.
#-------------------------------------
# TODO: Consider the use of FIND_PACKAGE here
INCLUDE(FindTIFF)
INCLUDE(FindBoost)
INCLUDE(FindZLIB)

# Find optional libs

# FLTK
IF(AQSIS_USE_FLTK)
	INCLUDE(FindFLTK)

	IF(NOT AQSIS_FLTK_FOUND)
		MESSAGE(STATUS "*** FLTK not found ***  building without eqsl, piqsl and legacy framebuffer")
		SET(AQSIS_USE_FLTK OFF)
	ENDIF(NOT AQSIS_FLTK_FOUND)

	IF(NOT AQSIS_FLUID_FOUND)
		MESSAGE(STATUS "*** fluid not found ***  building without eqsl and piqsl")
	ENDIF(NOT AQSIS_FLUID_FOUND)
ENDIF(AQSIS_USE_FLTK)

# OpenEXR
IF(AQSIS_USE_OPENEXR)
	INCLUDE(FindOpenEXR)

	IF(NOT AQSIS_OPENEXR_FOUND)
		MESSAGE(STATUS "Cannot find OpenEXR - aqsis will be build without support for the OpenEXR image format")
		SET(AQSIS_USE_OPENEXR OFF)
	ENDIF(NOT AQSIS_OPENEXR_FOUND)
ENDIF(AQSIS_USE_OPENEXR)

# JPEG
# libjpeg is no longer needed until aqsis again supports jpeg images.
#INCLUDE(FindJPEG)

# find tinyxml.  If not found we use the version distributed with the aqsis
# source.
IF(AQSIS_USE_EXTERNAL_TINYXML)
	INCLUDE(FindTinyXML)
	IF(NOT AQSIS_TINYXML_FOUND)
		MESSAGE(STATUS "Cannot find external tinyxml library - using version included with the aqsis source.")
		SET(AQSIS_USE_EXTERNAL_TINYXML OFF)
	ENDIF(NOT AQSIS_TINYXML_FOUND)
ENDIF(AQSIS_USE_EXTERNAL_TINYXML)


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
# Set program version numbers
#----------------------------

# These are needed by version.h, as well as the packaging stuff.  Note that
# some packaging variables are set in the platform-specific config files, and
# these version variables are potentially used there.
SET(MAJOR 1)
SET(MINOR 5)
SET(BUILD 0)
SVN_REPOSITORY_VERSION( REVISION ${PROJECT_SOURCE_DIR} )


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

# Set up the install rpath if desired.
SET(CMAKE_SKIP_RPATH OFF CACHE INTERNAL "Disabled, since we need RPATHS to find aqsl during the build" FORCE)
IF(AQSIS_USE_RPATH)
	SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${LIBDIR}")
ENDIF(AQSIS_USE_RPATH)

#-------------------------------------------------------------------------------
# Define macros which enable compile-time features
#-------------------------------------------------

ADD_DEFINITIONS(-DAQSIS_MAIN_CONFIG_NAME="${AQSIS_MAIN_CONFIG_NAME}")


#-------------------------------------------------------------------------------
# Create version.h
#-----------------
CONFIGURE_FILE(version.h.in.cmake ${PROJECT_BINARY_DIR}/version.h)
# Make sure the rest of the build can find version.h
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})


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
ADD_SUBDIRECTORY(rib/ribparse)
ADD_SUBDIRECTORY(rib/ri2rib)
ADD_SUBDIRECTORY(rib/miqser)
ADD_SUBDIRECTORY(texturing/teqser)
ADD_SUBDIRECTORY(displays/display)
ADD_SUBDIRECTORY(thirdparty/tinyxml)
ADD_SUBDIRECTORY(displays/d_exr)
ADD_SUBDIRECTORY(displays/d_xpm)
ADD_SUBDIRECTORY(displays/d_sdcBMP)
ADD_SUBDIRECTORY(shaders)
IF(AQSIS_USE_FLTK AND AQSIS_FLUID_FOUND AND AQSIS_BOOST_FOUND)
	ADD_SUBDIRECTORY(displays/piqsl)
	GET_DIRECTORY_PROPERTY(piqsl_DISPLAYLIB DIRECTORY displays/piqsl DEFINITION DISPLAYLIB)
ENDIF(AQSIS_USE_FLTK AND AQSIS_FLUID_FOUND AND AQSIS_BOOST_FOUND)
IF(AQSIS_USE_FLTK AND AQSIS_FLUID_FOUND)
	ADD_SUBDIRECTORY(tools/eqsl)
ENDIF(AQSIS_USE_FLTK AND AQSIS_FLUID_FOUND)
IF(AQSIS_USE_PDIFF)
	ADD_SUBDIRECTORY(thirdparty/pdiff)
ENDIF(AQSIS_USE_PDIFF)
ADD_SUBDIRECTORY(content)
ADD_SUBDIRECTORY(tools/mpdump)


#-------------------------------------------------------------------------------
# Build aqsisrc
#--------------
GET_DIRECTORY_PROPERTY(display_DISPLAYLIB DIRECTORY displays/display DEFINITION DISPLAYLIB)
GET_DIRECTORY_PROPERTY(d_exr_DISPLAYLIB DIRECTORY displays/d_exr DEFINITION DISPLAYLIB)
GET_DIRECTORY_PROPERTY(d_bmp_DISPLAYLIB DIRECTORY displays/d_sdcBMP DEFINITION DISPLAYLIB)
GET_DIRECTORY_PROPERTY(d_xpm_DISPLAYLIB DIRECTORY displays/d_xpm DEFINITION DISPLAYLIB)
# Default search paths.
SET_WITH_PATH_PREFIX(DEFAULT_SHADERPATH "${SHADERDIR}" "${CMAKE_INSTALL_PREFIX}")
SET(DEFAULT_ARCHIVEPATH "${CMAKE_INSTALL_PREFIX}")
SET(DEFAULT_TEXTUREPATH "${CMAKE_INSTALL_PREFIX}")
SET_WITH_PATH_PREFIX(DEFAULT_DISPLAYPATH "${DISPLAYSDIR}" "${CMAKE_INSTALL_PREFIX}")
SET(DEFAULT_PROCEDURALPATH "${CMAKE_INSTALL_PREFIX}")
SET(DEFAULT_RESOURCEPATH "${CMAKE_INSTALL_PREFIX}")
# Search paths which are settable from the cmake build.
SET(SHADERPATH "${SHADERPATH}" CACHE STRING "Aqsis shader searchpath")
SET(ARCHIVEPATH "${ARCHIVEPATH}" CACHE STRING "Aqsis archive searchpath")
SET(TEXTUREPATH "${TEXTUREPATH}" CACHE STRING "Aqsis texture searchpath")
SET(DISPLAYPATH "${DISPLAYPATH}" CACHE STRING "Aqsis display searchpath")
SET(PROCEDURALPATH "${PROCEDURALPATH}" CACHE STRING "Aqsis procedural searchpath")
SET(RESOURCEPATH "${RESOURCEPATH}" CACHE STRING "Aqsis resource searchpath")
CONFIGURE_FILE(aqsisrc.in.cmake ${PROJECT_BINARY_DIR}/aqsisrc)
ADD_CUSTOM_TARGET(aqsisrc ALL echo
	DEPENDS  ${PROJECT_BINARY_DIR}/aqsisrc
	)
INSTALL(FILES ${PROJECT_BINARY_DIR}/aqsisrc DESTINATION ${SYSCONFDIR})


#-------------------------------------------------------------------------------
# Packaging
#----------


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

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${AQSIS_PROJECT_NAME}")
SET(CPACK_PACKAGE_VENDOR "${AQSIS_PROJECT_VENDOR}")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
#SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${BUILD}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${AQSIS_PROJECT_NAME_SHORT}-${MAJOR}.${MINOR}")
INCLUDE(CPack)
