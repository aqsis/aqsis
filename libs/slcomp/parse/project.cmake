# Generate scanner by invoking flex with a custom command
set(_scanner_cpp_name ${parse_BINARY_DIR}/scanner.cpp)
add_custom_command(
	OUTPUT ${_scanner_cpp_name}
	COMMAND ${AQSIS_FLEX_EXECUTABLE}
		-o${_scanner_cpp_name} ${parse_SOURCE_DIR}/scanner.ll
	MAIN_DEPENDENCY ${parse_SOURCE_DIR}/scanner.ll
	DEPENDS ${parse_BINARY_DIR}/parser.hpp
)

# Generate parser by invoking bison with a custom command
set(_parser_cpp_name ${parse_BINARY_DIR}/parser.cpp)
set(_parser_hpp_name ${parse_BINARY_DIR}/parser.hpp)
if(WIN32 AND AQSIS_DEPENDENCIES)
	file(TO_NATIVE_PATH "${AQSIS_DEPENDENCIES}/bin" deps_bin_path)
	add_custom_command(
		OUTPUT ${_parser_cpp_name} ${_parser_hpp_name}
		COMMAND ${CMAKE_COMMAND} -Dbison_command=${AQSIS_BISON_EXECUTABLE}
								 -Ddeps_bin_path=${deps_bin_path}
								 -Dcpp_output_name=${_parser_cpp_name} 
								 -Dhpp_output_name=${_parser_hpp_name}
								 -Dparser_file=${parse_SOURCE_DIR}/parser.yy
								 -P ${CMAKE_SOURCE_DIR}/cmake/bison.cmake
		MAIN_DEPENDENCY ${parse_SOURCE_DIR}/parser.yy
	)
else()
	add_custom_command(
		OUTPUT ${_parser_cpp_name} ${_parser_hpp_name}
		COMMAND ${AQSIS_BISON_EXECUTABLE} 
			${AQSIS_BISON_ARGUMENTS}
			-d -o ${_parser_cpp_name} ${parse_SOURCE_DIR}/parser.yy 
		MAIN_DEPENDENCY ${parse_SOURCE_DIR}/parser.yy
	)
endif()

# Create source list variables
set(parse_srcs
	funcdef.cpp
	libslparse.cpp
	optimise.cpp
	parsenode.cpp
	typecheck.cpp
	vardef.cpp
)
set(parse_srcs ${parse_srcs} ${_scanner_cpp_name} ${_parser_cpp_name})
make_absolute(parse_srcs ${parse_SOURCE_DIR})

# Create header list variables
set(parse_hdrs
	funcdef.h
	parsenode.h
	vardef.h
)
set(parse_hdrs ${parse_hdrs} ${_parser_hpp_name})
make_absolute(parse_hdrs ${parse_SOURCE_DIR})

include_directories(${parse_SOURCE_DIR})
include_directories(${parse_BINARY_DIR})
