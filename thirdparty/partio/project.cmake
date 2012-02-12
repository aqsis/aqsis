# For now we just build partio into aqsis statically where necessary: It's a
# small library, and obscure enough to be unlikely to get into standard
# distribution channels in the forseeable future.
#
# The following defines the necessary compile flags and source files.

# First check whether the git submodule for partio has been initialized
if(NOT EXISTS ${partio_SOURCE_DIR}/src/src/lib)
    message(FATAL_ERROR
        "Partio repository not found: please initialize using\ngit submodule update --init")
endif()

file(GLOB partio_srcs
    "${partio_SOURCE_DIR}/src/src/lib/io/*.cpp"
    "${partio_SOURCE_DIR}/src/src/lib/core/*.cpp")

make_absolute(partio_srcs ${partio_SOURCE_DIR})

include_directories(${partio_SOURCE_DIR}/src/src/lib)
add_definitions(-DPARTIO_USE_ZLIB)
if(WIN32)
  add_definitions(-DPARTIO_WIN32)
endif()

include_directories(${AQSIS_ZLIB_INCLUDE_DIR})
set(partio_libs ${AQSIS_ZLIB_LIBRARIES})
