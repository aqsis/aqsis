set(ENV{PATH} "$ENV{PATH};${util_path};${slcomp_path}")
execute_process(COMMAND ${aqsl_command} -o ${shader_output_name} -I ${shader_include_path} ${shader_name})
