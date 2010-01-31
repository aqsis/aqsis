cmake_minimum_required(VERSION 2.6)

project(raster)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_path(ILMBASE_INCLUDE_DIR OpenEXR/ImathBox.h)
find_package(TIFF)
find_package(Boost 1.36.0 COMPONENTS unit_test_framework)

include_directories(${TIFF_INCLUDE_DIR})
include_directories(${ILMBASE_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(ustring)
include_directories(.)

set(srcs
    main.cpp
    renderer.cpp
    ustring/ustring.cpp
    fixedstrings.cpp
    primvar.cpp
    varspec.cpp
    scenes/tenpatch.cpp
    scenes/default.cpp
)

set(hdrs
	arrayview.h
    attributes.h
	fixedstrings.h
	geometry.h
	grid.h
	gridvar.h
	invbilin.h
	microquadsampler.h
	options.h
	pointinquad.h
	primvar.h
	renderer.h
	sample.h
	simple.h
	surfaces.h
	util.h
	varspec.h
)

source_group("Header Files" FILES ${hdrs})

add_executable(render ${srcs} ${hdrs})
target_link_libraries(render ${TIFF_LIBRARIES})

add_executable(primvar_test
    primvar_test.cpp
    ustring/ustring.cpp
    fixedstrings.cpp
    primvar.cpp
    varspec.cpp
)

if(WIN32)
    set_property(TARGET primvar_test PROPERTY COMPILE_DEFINITIONS OpenImageIO_EXPORTS)
    set_property(TARGET render PROPERTY COMPILE_DEFINITIONS OpenImageIO_EXPORTS)
endif(WIN32)

#target_link_libraries(varspec_test ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
