set(ENV{PATH} "$ENV{PATH};${deps_bin_path}")
execute_process(COMMAND ${bison_command} -d -o ${cpp_output_name} ${parser_file})
