cmake_minimum_required(VERSION 2.6)

project(raster)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_path(ILMBASE_INCLUDE_DIR OpenEXR/ImathBox.h)
find_package(TIFF)
find_package(Boost 1.36.0)

include_directories(${TIFF_INCLUDE_DIR})
include_directories(${ILMBASE_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})

set(srcs
    main.cpp
    renderer.cpp
    grid.cpp
)

add_executable(render ${srcs})

target_link_libraries(render ${TIFF_LIBRARIES})
