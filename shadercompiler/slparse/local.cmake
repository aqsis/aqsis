FILE(GLOB SLPARSE_SRCS ${SLPARSE_SOURCE_DIR}/*.cpp)
FILE(GLOB SLPARSE_HDRS ${SLPARSE_SOURCE_DIR}/*.h)

# Create target for the parser
ADD_CUSTOM_TARGET(slparser echo "Creating SL parser/scanner")

# Create custom command for flex/lex (note the outputs)
ADD_CUSTOM_COMMAND(
	SOURCE ${SLPARSE_SOURCE_DIR}/scanner.ll
	COMMAND ${FLEX_EXECUTABLE} 
	ARGS -o${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp
			${SLPARSE_SOURCE_DIR}/scanner.ll
	TARGET slparser
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp
	OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp)

# Create custom command for bison/yacc (note the DEPENDS)
ADD_CUSTOM_COMMAND(
	SOURCE ${SLPARSE_SOURCE_DIR}/parser.yy
	COMMAND ${BISON_EXECUTABLE} 
	ARGS -d ${SLPARSE_SOURCE_DIR}/parser.yy
		-o ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
	TARGET slparser
	OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
	OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp)

# Add the parser and scanner to the list of sources
SET(SLPARSE_SRCS ${SLPARSE_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
SET(SLPARSE_SRCS ${SLPARSE_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp)
SET(SLPARSE_HDRS ${SLPARSE_HDRS} ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp)

# Since parser.cpp and scanner.cpp do not exists yet when cmake is run, mark
# them as generated
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/parser.cpp ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp GENERATED)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_DEFINITIONS(-DSLPARSE_EXPORTS)

SOURCE_GROUP("Header files" FILES ${SLPARSE_HDRS})

