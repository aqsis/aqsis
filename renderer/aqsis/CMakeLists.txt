SET(AQSIS_SRCS aqsis.cpp)

# TODO: Need to use the link.cmake system.
LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})

INCLUDE(${AQSISTYPES_SOURCE_DIR}/export.cmake)
INCLUDE(${RIBAPI_SOURCE_DIR}/export.cmake)
INCLUDE(${RIB_SOURCE_DIR}/export.cmake)
INCLUDE(${RIB2RI_SOURCE_DIR}/export.cmake)
INCLUDE(${ARGPARSE_SOURCE_DIR}/export.cmake)

ADD_EXECUTABLE(aqsis ${AQSIS_SRCS})
TARGET_LINK_LIBRARIES(aqsis argparse aqsistypes aqsislib shadervm ${TIFF_LIBRARIES})
