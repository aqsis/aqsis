FILE(GLOB RIB_SRCS ${RIB_SOURCE_DIR}/*.cpp)
FILE(GLOB RIB_HDRS ${RIB_SOURCE_DIR}/*.h)

# Add the parser and scanner to the list of sources
SET(RIB_SRCS ${RIB_SRCS} ${RIB_BINARY_DIR}/parser.cpp)
SET(RIB_SRCS ${RIB_SRCS} ${RIB_BINARY_DIR}/scanner.cpp)
SET(RIB_HDRS ${RIB_HDRS} ${RIB_BINARY_DIR}/parser.hpp)

INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

# Since parser.cpp and scanner.cpp do not exists yet when cmake is run, mark
# them as generated
SET_SOURCE_FILES_PROPERTIES(${RIB_BINARY_DIR}/parser.cpp 
							${RIB_BINARY_DIR}/parser.hpp 
							${RIB_BINARY_DIR}/scanner.cpp 
							GENERATED)

INCLUDE_DIRECTORIES(${RIB_BINARY_DIR})

ADD_DEFINITIONS(-DRIB_EXPORTS)
