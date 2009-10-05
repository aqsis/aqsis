project(aqsis_all)

cmake_minimum_required(VERSION 2.6.3)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

# Disallow in-source build
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" aqsis_in_source)
if(aqsis_in_source)
	message(FATAL_ERROR "Aqsis requires an out of source build. Please create a separate build directory and run 'cmake path_to_source [options]' there.")
endif()


# Path to aqsis-specific include directories for cmake helper scripts.
set(CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include some helper scripts and macros.
include(UtilMacros)
include(AqsisTesting)
include(FirstCMakeRun)
include(XslTransform)

#-------------------------------------------------------------------------------
# Build-time options which can be set from ccmake or the command line
#--------------------------------------------------------------------

option(AQSIS_USE_TIMERS "Enable performance timers" ON)
option(AQSIS_USE_PDIFF "Build the external pdiff perceptual image difference utility" OFF)
option(AQSIS_USE_FLTK "Build the aqsis GUI components which rely on the fltk library" ON)
option(AQSIS_USE_OPENEXR "Build aqsis with support for the OpenEXR image format" ON)
option(AQSIS_USE_EXTERNAL_TINYXML "Try to find and use an external tinyxml library" OFF)
mark_as_advanced(AQSIS_USE_PDIFF AQSIS_USE_EXTERNAL_TINYXML)

option(aqsis_enable_testing "Enable unit testing" OFF)
option(AQSIS_ENABLE_MPDUMP "Enable micropolygon dumping code" OFF)
option(AQSIS_ENABLE_MASSIVE "Enable Massive support" ON)
option(AQSIS_ENABLE_SIMBIONT "Enable Simbiont(RM) support" ON)
option(AQSIS_ENABLE_THREADING "Enable multi-threading (EXPERIMENTAL)" OFF)
option(AQSIS_ENABLE_DEVDOCS "Enable developer documentation" OFF)
mark_as_advanced(AQSIS_ENABLE_MPDUMP AQSIS_ENABLE_MASSIVE AQSIS_ENABLE_SIMBIONT
	AQSIS_ENABLE_DEVDOCS)

option(AQSIS_USE_RPATH "Enable runtime path for installed libs" ON)
mark_as_advanced(AQSIS_USE_RPATH)

if(WIN32)
	# Find path to precompiled libs on windows.
	set(AQSIS_WIN32LIBS "" CACHE PATH "The location of the win32libs SVN folder")
endif()

set(AQSIS_MAIN_CONFIG_NAME "aqsisrc"
	CACHE STRING "Name of the main aqsis configuration file")
mark_as_advanced(AQSIS_MAIN_CONFIG_NAME)


#-------------------------------------------------------------------------------
# Find necessary library dependencies.
#-------------------------------------

# Required libs
# -------------
find_package(TIFF)
find_package(ZLIB)

# Find boost.
set(BOOST_ROOT "$ENV{BOOST_ROOT}" CACHE PATH "Root location of the Boost install")
if(WIN32)
	set(Boost_USE_STATIC_LIBS ON)
endif()
set(Boost_ADDITIONAL_VERSIONS "1.40.0" "1.40" "1.39.0" "1.39" "1.38.0" "1.38" "1.37.0" "1.37")
find_package(Boost 1.34.1)
# The following is a workaround because Boost versions > 1.35.0 need the
# system library. CMake versions >2.6.4 should fix this properly.
set(local_boost_version "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}")
if(local_boost_version VERSION_LESS "1.35")
    find_package(Boost 1.34.1
		COMPONENTS filesystem iostreams wave unit_test_framework thread regex)
else()
    find_package(Boost 1.34.1
		COMPONENTS filesystem iostreams wave unit_test_framework thread regex system)
endif()

link_directories(${Boost_LIBRARY_DIRS})


# Optional libs
# -------------

# FLTK
if(AQSIS_USE_FLTK)
	find_package(FLTK)

	if(NOT AQSIS_FLTK_FOUND)
		message("** FLTK not found - building without eqsl, piqsl and legacy framebuffer")
		set(AQSIS_USE_FLTK OFF)
	endif()

	if(NOT AQSIS_FLUID_FOUND)
		message("** fluid not found - building without eqsl and piqsl")
	endif()
endif()

# OpenEXR
if(AQSIS_USE_OPENEXR)
	find_package(OpenEXR)

	if(NOT AQSIS_OPENEXR_FOUND)
		message("** Cannot find OpenEXR - aqsis will be build without support for the OpenEXR image format")
		set(AQSIS_USE_OPENEXR OFF)
	endif()
endif()


## find tinyxml.  If not found we use the version distributed with the aqsis
## source.
#if(AQSIS_USE_EXTERNAL_TINYXML)
#	find_package(TinyXML)
#	if(NOT TINYXML_FOUND)
#		message("** Cannot find external tinyxml library - using version included with the aqsis source.")
#		set(AQSIS_USE_EXTERNAL_TINYXML OFF)
#	endif()
#endif()


#-------------------------------------------------------------------------------
# Find build tools
#-----------------

include(FindXsltproc)
if(NOT AQSIS_XSLTPROC_EXECUTABLE_FOUND)
	message(FATAL_ERROR "Aqsis requires xsltproc to build")
endif()

include(FindFlexBison)

if(NOT AQSIS_FLEX_EXECUTABLE_FOUND)
	message(FATAL_ERROR "Aqsis requires flex to build")
endif()
if(NOT AQSIS_BISON_EXECUTABLE_FOUND)
	message(FATAL_ERROR "Aqsis requires bison to build")
endif()

# Windows-specific build tools:
include(FindNSIS)
# Apple-specific build tools:
include(FindOsacompile)
include(FindXcodebuild)
include(FindResource)

#-------------------------------------------------------------------------------
# Set program version numbers
#----------------------------

# These are needed by version.h, as well as the packaging stuff.  Note that
# some packaging variables are set in the platform-specific config files, and
# these version variables are potentially used there.
set(VERSION_MAJOR 1)
set(VERSION_MINOR 6)
set(VERSION_BUILD 0)
set(SCM_REVISION 0) # "?"


#-------------------------------------------------------------------------------
# Set variables which control the build
#--------------------------------------

# Set convenience locations into which libraries and executables will be
# placed during the build.  Note: This is irrelevant to the install.
set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
#set(ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)


# Set default build type the first time cmake is run, but only if it's empty
# since an alternative value might have been specified on the command line,
# and we want this to override the default given below.
#
# We need to jump through some hoops here with the set_if_empty() macro, since
# CMAKE_BUILD_TYPE is a system-defined variable which means we can't get to it
# with set() before it's defined.
if(FIRST_CMAKE_RUN)
	set_if_empty(CMAKE_BUILD_TYPE "Release" CACHE STRING
		"Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
		FORCE
		)
endif()

# Set up the install rpath if desired.
set(CMAKE_SKIP_RPATH OFF CACHE INTERNAL
	"Disabled, since we need RPATHS to find aqsl during the build" FORCE)
if(AQSIS_USE_RPATH)
	set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${LIBDIR}")
endif()


#-------------------------------------------------------------------------------
# System-specific includes
#-------------------------

# Include setup for system-specific paths, packaging etc
if(WIN32)
	include(${CMAKE_SOURCE_DIR}/cmake/platform/windows.cmake)
	# Build the .rc files for library and executable version information and icons.
	set(INFORES_SRCS "${PROJECT_BINARY_DIR}/info.rc")
	set(ICONRES_SRCS "${PROJECT_BINARY_DIR}/icon.rc")
	configure_file("${CMAKE_SOURCE_DIR}/distribution/win/info.rc.in.cmake" ${INFORES_SRCS})
	configure_file("${CMAKE_SOURCE_DIR}/distribution/win/icon.rc.in.cmake" ${ICONRES_SRCS})
elseif(APPLE)
	include(${CMAKE_SOURCE_DIR}/cmake/platform/macosx.cmake)
else()
	include(${CMAKE_SOURCE_DIR}/cmake/platform/linux.cmake) 
endif()


#-------------------------------------------------------------------------------
# Add directories for all subprojects
#------------------------------------
add_subdirectory(include/aqsis)
include_directories(
	"${PROJECT_SOURCE_DIR}/include"
	"${PROJECT_BINARY_DIR}/include"
	${Boost_INCLUDE_DIRS}
)

# Thirdparty libs
declare_subproject(thirdparty/tinyxml)
if(AQSIS_USE_PDIFF)
  add_subdirectory(thirdparty/pdiff)
endif()

# Build libraries
add_subdirectory(libs/math)
add_subdirectory(libs/util)
get_directory_property(aqsis_util_location DIRECTORY libs/util DEFINITION aqsis_util_location)
add_subdirectory(libs/riutil)
add_subdirectory(libs/ribparse)
add_subdirectory(libs/slcomp)
get_directory_property(aqsis_slcomp_location DIRECTORY libs/slcomp DEFINITION aqsis_slcomp_location)
add_subdirectory(libs/tex)
add_subdirectory(libs/shadervm)
add_subdirectory(libs/slxargs)
add_subdirectory(libs/ri2rib)
add_subdirectory(libs/rib2ri)
add_subdirectory(libs/core)

# Build executable tools
add_subdirectory(tools/aqsl)
add_subdirectory(tools/aqsltell)
add_subdirectory(tools/aqsis)
add_subdirectory(tools/miqser)
add_subdirectory(tools/teqser)
if(AQSIS_USE_FLTK AND AQSIS_FLUID_FOUND)
	add_subdirectory(tools/eqsl)
	add_subdirectory(tools/piqsl)
endif()

# Build displays
add_subdirectory(tools/displays)

# Build additional plugins
add_subdirectory(tools/procedurals/hairgen)
#add_subdirectory(tools/shadeops/sci_volumes)
add_subdirectory(tools/neqsus)

# Build shaders & install examples
add_subdirectory(shaders)
add_subdirectory(examples)

# Build docs
if(AQSIS_ENABLE_DEVDOCS)
	add_subdirectory(doc/doxygen)
endif()

# Packaging
add_subdirectory(distribution)

#-------------------------------------------------------------------------------
# Create aqsisrc
#--------------
get_directory_property(display_DISPLAYLIB DIRECTORY tools/displays DEFINITION file_display_name)
get_directory_property(d_exr_DISPLAYLIB DIRECTORY tools/displays DEFINITION exr_display_name)
get_directory_property(d_bmp_DISPLAYLIB DIRECTORY tools/displays DEFINITION bmp_display_name)
get_directory_property(d_xpm_DISPLAYLIB DIRECTORY tools/displays DEFINITION xpm_display_name)
get_directory_property(piqsl_DISPLAYLIB DIRECTORY tools/displays DEFINITION piqsl_display_name)
# Default search paths.
set(DEFAULT_SHADERPATH ${shader_search_path})
set(DEFAULT_ARCHIVEPATH "${CMAKE_INSTALL_PREFIX}")
set(DEFAULT_TEXTUREPATH "${CMAKE_INSTALL_PREFIX}")
set_with_path_prefix(DEFAULT_DISPLAYPATH "${PLUGINDIR}" "${CMAKE_INSTALL_PREFIX}")
set_with_path_prefix(DEFAULT_PROCEDURALPATH "${PLUGINDIR}" "${CMAKE_INSTALL_PREFIX}")
set(DEFAULT_RESOURCEPATH "${CMAKE_INSTALL_PREFIX}")
# Search paths which are settable from the cmake build.
set(SHADERPATH "${SHADERPATH}" CACHE STRING "Aqsis shader searchpath")
set(ARCHIVEPATH "${ARCHIVEPATH}" CACHE STRING "Aqsis archive searchpath")
set(TEXTUREPATH "${TEXTUREPATH}" CACHE STRING "Aqsis texture searchpath")
set(DISPLAYPATH "${DISPLAYPATH}" CACHE STRING "Aqsis display searchpath")
set(PROCEDURALPATH "${PROCEDURALPATH}" CACHE STRING "Aqsis procedural searchpath")
set(RESOURCEPATH "${RESOURCEPATH}" CACHE STRING "Aqsis resource searchpath")
set(aqsisrc_name ${PROJECT_BINARY_DIR}/aqsisrc)
configure_file(aqsisrc.in.cmake ${aqsisrc_name})
install(FILES ${aqsisrc_name} DESTINATION ${SYSCONFDIR} COMPONENT main)

