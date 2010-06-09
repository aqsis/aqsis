cmake_minimum_required(VERSION 2.6)

project(raster)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_path(ILMBASE_INCLUDE_DIR OpenEXR/ImathBox.h)
find_library(ILMBASE_IEX_LIBRARY Iex)

find_package(TIFF)
find_package(Boost 1.36.0 COMPONENTS unit_test_framework)

include_directories(${TIFF_INCLUDE_DIR})
include_directories(${ILMBASE_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(ustring)
include_directories(.)

if(UNIX)
    if("${CMAKE_CXX_FLAGS}" STREQUAL "")
        set(CMAKE_CXX_FLAGS "-Wall -Werror" CACHE STRING
            "Flags used by the compiler during all build types." FORCE)
    endif()
endif()

set(srcs
    filters.cpp
    main.cpp
    primvar.cpp
    renderer.cpp
    samplegen.cpp
    samplestorage.cpp
    scenes/default.cpp
    scenes/dofamounttest.cpp
    scenes/simpledeformation.cpp
    scenes/mbnoisetest.cpp
    scenes/tenpatch.cpp
    shader.cpp
    ustring/ustring.cpp
    varspec.cpp
)

set(hdrs
    arrayview.h
    attributes.h
    filters.h
    geometry.h
    grid.h
    gridstorage.h
    invbilin.h
    microquadsampler.h
    options.h
    pointinquad.h
    primvar.h
    renderer.h
    sample.h
    samplegen.h
    samplestorage.h
    scenes/scenes.h
    shader.h
    simple.h
    surfaces.h
    util.h
    varspec.h
)

source_group("Header Files" FILES ${hdrs})

add_executable(render ${srcs} ${hdrs})
target_link_libraries(render ${TIFF_LIBRARIES} ${ILMBASE_IEX_LIBRARY})


#------------------------------------------------------------
# Testing:

enable_testing()

macro(testcase testname)
    add_executable(${testname} ${ARGN})
    target_link_libraries(${testname} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
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

if(WIN32)
    set_property(TARGET primvar_test PROPERTY COMPILE_DEFINITIONS OpenImageIO_EXPORTS)
    set_property(TARGET render PROPERTY COMPILE_DEFINITIONS OpenImageIO_EXPORTS)
endif()

#target_link_libraries(varspec_test ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
