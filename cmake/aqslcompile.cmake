set(ENV{PATH} "$ENV{PATH};${util_path};${slcomp_path};${shared_lib_path}")
separate_arguments(RSL_COMPILE_FLAGS)
execute_process(COMMAND ${RSL_COMPILER} ${RSL_COMPILE_FLAGS} ${RSL_DEPEND_FLAGS} 
										-o ${RSL_SHADER} 
										${RSL_SOURCE})
