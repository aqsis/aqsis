PROJECT(AQSLTELL)

FILE(GLOB AQSLTELL_SRCS ${AQSLTELL_SOURCE_DIR}/*.cpp)
FILE(GLOB AQSLTELL_HDRS ${AQSLTELL_SOURCE_DIR}/*.h)

# TODO: Need to use the link.cmake system.
LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})

INCLUDE(${AQSISTYPES_SOURCE_DIR}/export.cmake)
INCLUDE(${ARGPARSE_SOURCE_DIR}/export.cmake)
INCLUDE(${SLXARGS_SOURCE_DIR}/export.cmake)

ADD_DEFINITIONS(-DDEFAULT_SHADER_PATH="\\"/usr/local/aqsis/share/shaders\\"")

ADD_EXECUTABLE(aqsltell ${AQSLTELL_SRCS})
IF(WIN32)
	TARGET_LINK_LIBRARIES(aqsltell argparse aqsistypes slxargs shadervm)
ELSE(WIN32)
	TARGET_LINK_LIBRARIES(aqsltell argparse aqsistypes slxargs shadervm)
ENDIF(WIN32)
