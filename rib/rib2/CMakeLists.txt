PROJECT(RIB)


# Create custom command for flex/lex (note the outputs)
ADD_CUSTOM_COMMAND(
	SOURCE ${RIB_SOURCE_DIR}/scanner.ll
	COMMAND ${AQSIS_FLEX_EXECUTABLE} 
	ARGS -o${RIB_BINARY_DIR}/scanner.cpp
			${RIB_SOURCE_DIR}/scanner.ll
	TARGET ribparser
	DEPENDS ${RIB_BINARY_DIR}/parser.hpp
	OUTPUTS ${RIB_BINARY_DIR}/scanner.cpp)

# Create custom command for bison/yacc (note the DEPENDS)
ADD_CUSTOM_COMMAND(
	SOURCE ${RIB_SOURCE_DIR}/parser.yy
	COMMAND ${AQSIS_BISON_EXECUTABLE} 
	ARGS -d ${RIB_SOURCE_DIR}/parser.yy
		-o ${RIB_BINARY_DIR}/parser.cpp
	TARGET ribparser
	OUTPUTS ${RIB_BINARY_DIR}/parser.cpp
	OUTPUTS ${RIB_BINARY_DIR}/parser.hpp)

# Create target for the parser
ADD_CUSTOM_TARGET(ribparser echo "Creating RIB parser/scanner"
		DEPENDS ${RIB_BINARY_DIR}/parser.cpp
				${RIB_BINARY_DIR}/scanner.cpp
				${RIB_BINARY_DIR}/parser.hpp)

SOURCE_GROUP("Parser Source" FILES ${RIB_SOURCE_DIR}/parser.yy)
SOURCE_GROUP("Scanner Source" FILES ${RIB_SOURCE_DIR}/scanner.ll)
