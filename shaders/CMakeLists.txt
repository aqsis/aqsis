PROJECT(SHADERS)

FILE(GLOB SHADER_SRCS ${PROJECT_SOURCE_DIR}/*.sl)

SET(COMPILED_SHADERS)
FOREACH(SHADER ${SHADER_SRCS})
	# Get the base name of the shader, to work out the compiled name.
	# Note: We can't presume that the shader name (in the source) matches the 
	# shader filename, so we force the output. This is necessary so that we can
	# work out what the output name is for the dependecy checking.
	GET_FILENAME_COMPONENT(SHADER_BASENAME ${SHADER} NAME_WE)
	SET(SHADER_OUTPUT_NAME ${PROJECT_BINARY_DIR}/${SHADER_BASENAME}.slx)
	SET(COMPILED_SHADERS ${COMPILED_SHADERS} ${SHADER_OUTPUT_NAME})
	ADD_CUSTOM_COMMAND( 
		SOURCE ${SHADER}
		COMMAND ${AQSL_EXECUTABLE} 
		ARGS -o ${SHADER_OUTPUT_NAME} ${SHADER}
		TARGET shaders
		OUTPUTS ${SHADER_OUTPUT_NAME}
		DEPENDS ${SHADER} ${AQSL_EXECUTABLE}
		COMMENT "Building shader ${SHADER}"
	)
ENDFOREACH(SHADER)

# Add a custom target, dependent on the ALL target, that will
# force the shaders to be compiled.
ADD_CUSTOM_TARGET(shaders ALL DEPENDS ${COMPILED_SHADERS})

INSTALL(FILES ${SHADER_SRCS} DESTINATION ${SHADERDIR})
INSTALL(FILES ${COMPILED_SHADERS} DESTINATION ${SHADERDIR})
