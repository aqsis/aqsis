FILE(GLOB SLPARSE_SRCS ${SLPARSE_SOURCE_DIR}/*.cpp)
FILE(GLOB SLPARSE_HDRS ${SLPARSE_SOURCE_DIR}/*.h)

# Add the parser and scanner to the list of sources
SET(SLPARSE_SRCS ${SLPARSE_SRCS} ${SLPARSE_BINARY_DIR}/parser.cpp)
SET(SLPARSE_SRCS ${SLPARSE_SRCS} ${SLPARSE_BINARY_DIR}/scanner.cpp)
SET(SLPARSE_HDRS ${SLPARSE_HDRS} ${SLPARSE_BINARY_DIR}/parser.hpp)

# Since parser.cpp and scanner.cpp do not exists yet when cmake is run, mark
# them as generated
SET_SOURCE_FILES_PROPERTIES(${SLPARSE_BINARY_DIR}/parser.cpp 
							${SLPARSE_BINARY_DIR}/parser.hpp 
							${SLPARSE_BINARY_DIR}/scanner.cpp 
							GENERATED)

INCLUDE_DIRECTORIES(${SLPARSE_BINARY_DIR})

ADD_DEFINITIONS(-DSLPARSE_EXPORTS)
