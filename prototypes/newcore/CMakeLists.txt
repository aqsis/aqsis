cmake_minimum_required(VERSION 2.6)

project(raster)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_path(ILMBASE_INCLUDE_DIR OpenEXR/ImathBox.h)
find_package(TIFF)

include_directories(${ILMBASE_INCLUDE_DIR})

add_executable(render main.cpp)

target_link_libraries(render ${TIFF_LIBRARIES})
