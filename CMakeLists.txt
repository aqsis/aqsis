project(aqsis_all)

cmake_minimum_required(VERSION 2.6.3)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

# Disallow in-source build
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" aqsis_in_source)
if(aqsis_in_source)
	message(FATAL_ERROR "Aqsis requires an out of source build.  Please clean any cmake-generated files from the source directory, create a separate build directory and run 'cmake path_to_source [options]' there.")
endif()


# Path to aqsis-specific include directories for cmake helper scripts.
set(CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include some helper scripts and macros.
include(UtilMacros)
include(AqsisTesting)
include(FirstCMakeRun)

#-------------------------------------------------------------------------------
# Build-time options which can be set from ccmake or the command line
#--------------------------------------------------------------------

option(AQSIS_USE_TIMERS "Enable performance timers" ON)
option(AQSIS_USE_PDIFF "Build the external pdiff perceptual image difference utility" OFF)
option(AQSIS_USE_QT "Build the aqsis GUI components which rely on the Qt libraries" ON)
option(AQSIS_USE_OPENEXR "Build aqsis with support for the OpenEXR image format" ON)
option(AQSIS_USE_PNG "Build aqsis with support for reading PNG image files" ON)
option(AQSIS_USE_EXTERNAL_TINYXML "Try to find and use an external tinyxml library" OFF)
mark_as_advanced(AQSIS_USE_PDIFF AQSIS_USE_EXTERNAL_TINYXML)

option(aqsis_enable_testing "Enable unit testing" OFF)
option(AQSIS_ENABLE_MPDUMP "Enable micropolygon dumping code" OFF)
option(AQSIS_ENABLE_MASSIVE "Enable Massive support" ON)
option(AQSIS_ENABLE_SIMBIONT "Enable Simbiont(RM) support" ON)
option(AQSIS_ENABLE_THREADING "Enable multi-threading (EXPERIMENTAL)" OFF)
option(AQSIS_ENABLE_DOCS "Enable documentation generation" ON)
mark_as_advanced(AQSIS_ENABLE_MPDUMP AQSIS_ENABLE_MASSIVE AQSIS_ENABLE_SIMBIONT)

option(AQSIS_USE_RPATH "Enable runtime path for installed libs" ON)
mark_as_advanced(AQSIS_USE_RPATH)

if(WIN32)
	# Find path to precompiled libs on windows.
	set(AQSIS_WIN32LIBS "" CACHE PATH "The location of the win32libs SVN folder")
	if(MINGW)
		set(AQSIS_DEPENDENCIES "${CMAKE_SOURCE_DIR}/../dependencies" CACHE PATH "The location of the pre-built dependencies")
	endif()
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
if(WIN32)
	set(BOOST_ROOT "${AQSIS_DEPENDENCIES}" CACHE PATH "Root location of the boost install")
  if(NOT MINGW)
    set(Boost_USE_STATIC_LIBS ON)
  endif()
else()
	set(BOOST_ROOT "$ENV{BOOST_ROOT}" CACHE PATH "Root location of the Boost install")
endif()
set(Boost_ADDITIONAL_VERSIONS "1.45.0" "1.45" "1.44.0" "1.44"
	"1.43.0" "1.43" "1.42.0" "1.42" "1.41.0" "1.41"
	"1.40.0" "1.40" "1.39.0" "1.39" "1.38.0" "1.38" "1.37.0" "1.37")
find_package(Boost 1.34.1)
# The following is a workaround because Boost versions > 1.35.0 need the
# system library. CMake versions >2.6.4 should fix this properly.
set(local_boost_version "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}")
if(local_boost_version VERSION_LESS "1.35")
    if(WIN32)
        find_package(Boost 1.34.1
		COMPONENTS filesystem iostreams wave unit_test_framework thread regex zlib program_options)
    else()
        find_package(Boost 1.34.1
		COMPONENTS filesystem iostreams wave unit_test_framework thread regex program_options)
    endif()
else()
    if(WIN32)
        find_package(Boost 1.34.1
		COMPONENTS filesystem iostreams wave unit_test_framework thread regex system zlib program_options)
    else()
        find_package(Boost 1.34.1
		COMPONENTS filesystem iostreams wave unit_test_framework thread regex system program_options)
    endif()
endif()

link_directories(${Boost_LIBRARY_DIRS})


# Optional libs
# -------------

# GUI libs
if(AQSIS_USE_QT)
	find_package(Qt4 4.6.2 COMPONENTS QtCore QtGui QtOpenGL)

	if(NOT QT_FOUND)
		message("** Qt4 not found - building without aqsis GUI tools")
		set(AQSIS_USE_QT OFF)
	endif()
endif()

# OpenEXR
if(AQSIS_USE_OPENEXR)
	find_package(OpenEXR)

	if(NOT AQSIS_OPENEXR_FOUND)
		message("** Cannot find OpenEXR - aqsis will be built without support for the OpenEXR image format")
		set(AQSIS_USE_OPENEXR OFF)
	endif()
endif()

if(AQSIS_USE_PNG)
	find_package(PNG)

	if(NOT AQSIS_PNG_FOUND)
		message("** Cannot find PNG - aqsis will be built without support for the PNG image format")
		set(AQSIS_USE_PNG OFF)
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
# Find documentation tools
#-------------------------

if(AQSIS_ENABLE_DOCS)
	include(FindSphinx)

	find_package(Doxygen)

	if(NOT AQSIS_SPHINX_EXECUTABLE_FOUND)
                message("** Cannot find Sphinx - building without generating relevant documentation")
        endif()

	if(NOT DOXYGEN_FOUND)
		message("** Cannot find Doxygen - building without generating relevant documentation")
	endif()

	if(NOT AQSIS_SPHINX_EXECUTABLE_FOUND AND NOT DOXYGEN_FOUND)
		set(AQSIS_ENABLE_DOCS OFF)
	endif()
endif()


#-------------------------------------------------------------------------------
# Set program version numbers
#----------------------------

# These are needed by version.h, as well as the packaging stuff.  Note that
# some packaging variables are set in the platform-specific config files, and
# these version variables are potentially used there.
set(VERSION_MAJOR 1)
set(VERSION_MINOR 9)
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

# Make sure that CMAKE_INSTALL_PREFIX is absolute.  If we don't do this, it
# seems that we get relative values for the RPATH (which making
# CMAKE_INSTALL_RPATH absolute doesn't help??), which results in broken
# binaries.
if(NOT IS_ABSOLUTE ${CMAKE_INSTALL_PREFIX})
	message(STATUS "Warning: CMAKE_INSTALL_PREFIX relative path interpreted relative to build directory location.")
	set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}")
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
declare_subproject(thirdparty/partio)
declare_subproject(libs/pointrender)
if(AQSIS_USE_PDIFF)
  add_subdirectory(thirdparty/pdiff)
endif()

# Build libraries
add_subdirectory(libs/math)
add_subdirectory(libs/util)
get_directory_property(aqsis_util_location DIRECTORY libs/util DEFINITION aqsis_util_location)
add_subdirectory(libs/riutil)
add_subdirectory(libs/slcomp)
get_directory_property(aqsis_slcomp_location DIRECTORY libs/slcomp DEFINITION aqsis_slcomp_location)
add_subdirectory(libs/tex)
add_subdirectory(libs/shadervm)
add_subdirectory(libs/slxargs)
add_subdirectory(libs/ri2rib)
add_subdirectory(libs/core)

# Build executable tools
add_subdirectory(tools/aqsl)
add_subdirectory(tools/aqsltell)
add_subdirectory(tools/aqsis)
add_subdirectory(tools/miqser)
add_subdirectory(tools/teqser)
if(AQSIS_USE_QT)
	add_subdirectory(tools/eqsl)
	add_subdirectory(tools/piqsl)
	add_subdirectory(tools/ptview)
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

# Build documentation
if(AQSIS_ENABLE_DOCS)
	add_subdirectory(doc)
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

#-------------------------------------------------------------------------------
# Generate an AqsisConfig.cmake file, for use by projects that want to link
# against in the build tree rather than after installation.
#-------------------------------------------------------------------------------

set(AQSISCONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR}/AqsisConfig.cmake)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/AqsisConfig.cmake.in ${AQSISCONFIG_PATH})
export(TARGETS aqsis_core aqsis_math aqsis_shadervm aqsis_tex aqsis_util aqsis_riutil APPEND FILE ${AQSISCONFIG_PATH})

