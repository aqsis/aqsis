cmake_minimum_required(VERSION 2.6)

project(raster)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Determine whether this is the first cmake run.
if(NOT aqsis_not_first_cmake_run)
    SET(first_cmake_run ON)
    SET(aqsis_not_first_cmake_run ON CACHE INTERNAL
        "Indicate that this is not the first CMake run" FORCE)
else()
    SET(first_cmake_run OFF)
endif()

set(aqsis_enable_testing TRUE CACHE BOOL "Enable unit testing")
set(aqsis_memory_debugging FALSE CACHE BOOL "Enable memory debugging")
set(aqsis_use_threads TRUE CACHE BOOL "Enable threading")
set(aqsis_interactive_demo FALSE CACHE BOOL "Build interactive demo program")
if(first_cmake_run AND CMAKE_BUILD_TYPE STREQUAL "")
    # Make sure we build with the Release configuration by default.
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE
    )
endif()

find_path(ILMBASE_INCLUDE_DIR OpenEXR/ImathBox.h)
find_library(ILMBASE_IEX_LIBRARY Iex)

find_package(Qt4)
find_package(TIFF)
# Find boost.
if(WIN32)
	set(BOOST_ROOT "${AQSIS_DEPENDENCIES}" CACHE PATH "Root location of the boost install")
	set(Boost_USE_STATIC_LIBS ON)
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
		COMPONENTS unit_test_framework program_options thread)
    else()
        find_package(Boost 1.34.1
		COMPONENTS unit_test_framework program_options thread)
    endif()
else()
    if(WIN32)
        find_package(Boost 1.34.1
		COMPONENTS unit_test_framework program_options thread system)
    else()
        find_package(Boost 1.34.1
		COMPONENTS unit_test_framework program_options thread system)
    endif()
endif()

# Assumes aqsis has been built into the directory path/to/src/../build.  Can be
# overridden using Aqsis_DIR.
find_package(Aqsis PATHS ../../../build/)

include_directories(${TIFF_INCLUDE_DIR})
include_directories(${ILMBASE_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(ustring)
include_directories(.)
include_directories(${Aqsis_INCLUDE_DIRECTORIES})
if(aqsis_interactive_demo)
	include_directories(${QT_INCLUDES})
endif()

if(UNIX)
    if("${CMAKE_CXX_FLAGS}" STREQUAL "")
        set(CMAKE_CXX_FLAGS "-Wall -Werror" CACHE STRING
            "Flags used by the compiler during all build types." FORCE)
    endif()
endif()

set(srcs
    api.cpp
    displaymanager.cpp
    filterprocessor.cpp
    filters.cpp
    memdebug.cpp
    primvar.cpp
    renderer.cpp
    samplegen.cpp
    #scenes/default.cpp
    #scenes/dofamounttest.cpp
    #scenes/simpledeformation.cpp
    #scenes/mbnoisetest.cpp
    #scenes/tenpatch.cpp
    shader.cpp
    surfaces.cpp
    tessellation.cpp
    ustring/ustring.cpp
    varspec.cpp
    #tbb/tbb_misc.cpp
)

if(aqsis_memory_debugging)
    set_property(SOURCE memdebug.cpp
        PROPERTY COMPILE_DEFINITIONS AQSIS_MEMORY_DEBUGGING)
endif()

set(hdrs
    arrayview.h
    attributes.h
    filters.h
    filterprocessor.h
    geometry.h
    grid.h
    gridstorage.h
    invbilin.h
    microquadsampler.h
    options.h
    pointinquad.h
    primvar.h
    refcount.h
    renderer.h
    sample.h
    samplegen.h
    scenes/scenes.h
    shader.h
    surfaces.h
    util.h
    varspec.h
)

source_group("Header Files" FILES ${hdrs})

add_library(newcore STATIC ${srcs} ${hdrs})
add_executable(render main.cpp)

set(render_link_libs newcore ${TIFF_LIBRARIES} ${ILMBASE_IEX_LIBRARY}
                     ${Boost_PROGRAM_OPTIONS_LIBRARY} aqsis_riutil aqsis_math)
if(aqsis_use_threads)
    add_definitions(-DAQSIS_USE_THREADS)
    list(APPEND render_link_libs ${Boost_THREAD_LIBRARY})
endif()
target_link_libraries(render ${render_link_libs})

if(aqsis_interactive_demo)
	INCLUDE(${QT_USE_FILE})

	if(QT_FOUND)
		set(interactive_moc_hdrs interactive.h)
		QT4_WRAP_CPP(interactive_moc_srcs ${interactive_moc_hdrs})
		add_executable(interactive ${interactive_moc_srcs} interactive.cpp)
		target_link_libraries(interactive ${render_link_libs} ${QT_LIBRARIES})
	endif()
endif()


#------------------------------------------------------------
# Testing:

if(aqsis_enable_testing)

enable_testing()

macro(testcase testname)
    add_executable(${testname} ${ARGN})
    target_link_libraries(${testname} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
	if(WIN32)
		set_property(TARGET ${testname} PROPERTY COMPILE_DEFINITIONS OpenImageIO_EXPORTS BOOST_ALL_NO_LIB)
	endif()
    add_test(${testname} ${testname})
endmacro()

testcase(ustring_test
    ustring/ustring.cpp
    ustring/ustring_test.cpp
)

set(varspec_srcs
    ustring/ustring.cpp
    varspec.cpp
)

#add_executable(primvar_test ${varspec_srcs} primvar_test.cpp primvar.cpp)

testcase(varspec_test ${varspec_srcs} varspec_test.cpp)

testcase(gridstorage_test ${varspec_srcs} gridstorage_test.cpp)

testcase(arrayview_test arrayview_test.cpp)

testcase(util_test util_test.cpp)

testcase(samplegen_test samplegen.cpp samplegen_test.cpp)
target_link_libraries(samplegen_test ${Boost_THREAD_LIBRARY})

#testcase(splitstore_test splitstore_test.cpp)
#target_link_libraries(splitstore_test ${Boost_THREAD_LIBRARY})

testcase(treearraystorage_test treearraystorage_test.cpp)

if(WIN32)
	#set_property(TARGET primvar_test PROPERTY COMPILE_DEFINITIONS OpenImageIO_EXPORTS)
    set_property(TARGET render PROPERTY COMPILE_DEFINITIONS OpenImageIO_EXPORTS)
    set_property(TARGET newcore PROPERTY COMPILE_DEFINITIONS OpenImageIO_EXPORTS)
endif()

endif(aqsis_enable_testing)

#target_link_libraries(varspec_test ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
