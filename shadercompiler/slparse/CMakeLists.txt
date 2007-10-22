PROJECT(SLPARSE)


# Create custom command for flex/lex (note the outputs)
ADD_CUSTOM_COMMAND(
	SOURCE ${SLPARSE_SOURCE_DIR}/scanner.ll
	COMMAND ${AQSIS_FLEX_EXECUTABLE} 
	ARGS -o${SLPARSE_BINARY_DIR}/scanner.cpp
			${SLPARSE_SOURCE_DIR}/scanner.ll
	TARGET slparser
	DEPENDS ${SLPARSE_BINARY_DIR}/parser.hpp
	OUTPUTS ${SLPARSE_BINARY_DIR}/scanner.cpp)

# Create custom command for bison/yacc (note the DEPENDS)
ADD_CUSTOM_COMMAND(
	SOURCE ${SLPARSE_SOURCE_DIR}/parser.yy
	COMMAND ${AQSIS_BISON_EXECUTABLE} 
	ARGS -d ${SLPARSE_SOURCE_DIR}/parser.yy
		-o ${SLPARSE_BINARY_DIR}/parser.cpp
	TARGET slparser
	OUTPUTS ${SLPARSE_BINARY_DIR}/parser.cpp
	OUTPUTS ${SLPARSE_BINARY_DIR}/parser.hpp)

# Create target for the parser
ADD_CUSTOM_TARGET(slparser echo "Creating SL parser/scanner"
		DEPENDS ${SLPARSE_BINARY_DIR}/parser.cpp
				${SLPARSE_BINARY_DIR}/parser.hpp
				${SLPARSE_BINARY_DIR}/scanner.cpp)
