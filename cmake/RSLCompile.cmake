# @file
#
# Copyright 2009 Rising Sun Pictures Pty and the other authors and
# contributors. All Rights Reserved.
#
# @author Malcolm Humphreys
#
# @par License:
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# * Neither the name of the software's owners nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# (This is the Modified BSD License)

### Simple Example ###
# build some shaders with some external includes
#
#  # compile and install the renderman shaders
#  file(GLOB slSources "src/rman/*.sl")
#  add_rslshaders(rmanShaders ${slSources}
#      COMPILE_FLAGS ${SHADERLIB_COMPILE_FLAGS} ${CHEESE_COMPILE_FLAGS}
#      STAGE_DESTINATION ${STAGE_EXEC_PREFIX}/shaders)
#  install(
#      FILES ${rmanShaders_OUTPUT}
#      DESTINATION ${INSTALL_EXEC_PREFIX}/shaders
#      COMPONENT rmanShaders)

### DSO Depends Example ###
# builds the shaders after the DSO target is built
#
#  # compile and install the renderman shaders
#  file(GLOB slSources "src/rman/*.sl")
#  add_rslshaders(rmanShaders ${slSources}
#      COMPILE_FLAGS -DMY_FAV_DEF
#      DEPENDS ${DSO_NAME})
#  install(
#      FILES ${rmanShaders_OUTPUT}
#      DESTINATION ${INSTALL_EXEC_PREFIX}/shaders
#      COMPONENT rmanShaders)
#  install(
#      FILES ${slSources}
#      DESTINATION ${INSTALL_EXEC_PREFIX}/shaders/src
#      COMPONENT rmanShaders)

#
# todo: add some smarts to support other rman renderers
# eg. (prman, 3delight, pixie, air)
#
# The compiler is found by first looking in the variable RSL_COMPILER; if
# that's empty, we attempt to fill it in by looking in the path for a program
# called RSL_COMPILER_NAME
#
set(RSL_COMPILER_NAME aqsl)
set(RSL_OUTPUT_EXTENSION slx)
set(RSL_COMPILE_FLAGS -DAQSIS)

#
# Default verbosity
#
set(RSL_SETUP_VERBOSE YES)

#
# Defualt stage dir
#
set(RSL_STAGE_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/rslshaders")

# Include PARSE_ARGUMENTS macro
# http://www.itk.org/Wiki/CMakeMacroParseArguments
include(ParseArguments)

#
# Compile renderman shaders (.sl) MACRO
#
# Usage:
# add_rslshaders(target source1 source2 ...
#   [COMPILER shader_compiler]
#   [COMPILE_FLAGS flag1 flag2 ...]
#   [DEPENDS target1 target2 ...]
#   [STAGE_DESTINATION stage_path]
#   )
#
# Output:
# A variable ${target}_OUTPUT is created which will contain
# the compiled shaders file names.
#
# install(
#     FILES ${myTarget_OUTPUT} ${myTarget_SRC_OUTPUT}
#     DESTINATION ${SOME_SHADER_DIR})

#
MACRO(add_rslshaders RSL_TARGET)

    # parse the macro arguments
    PARSE_ARGUMENTS(RSL_USER
      "COMPILE_FLAGS;STAGE_DESTINATION;DEPENDS;"
      "NOTUSED" ${ARGN})

    # get the list of sources from the args
    set(RSL_SOURCES ${RSL_USER_DEFAULT_ARGS})

    if(RSL_SETUP_VERBOSE)
      message(STATUS "Setting up RSL target [${RSL_TARGET}]")
    endif(RSL_SETUP_VERBOSE)
    set(_rslshaders_debug OFF)

    # find the shader compiler
    if(NOT RSL_COMPILER)
      find_program(RSL_COMPILER ${RSL_COMPILER_NAME})
    endif()

    # merge the user compile flags with the build systems ones
    set(RSL_COMPILE_FLAGS ${RSL_COMPILE_FLAGS} ${RSL_USER_COMPILE_FLAGS})

    # include any dependent targets LIBRARY_OUTPUT_DIRECTORY
    set(RSL_DEPEND_FLAGS)
    foreach(_sl_dep ${RSL_USER_DEPENDS})
        get_target_property(_dep ${_sl_dep} LIBRARY_OUTPUT_DIRECTORY)
        set(RSL_DEPEND_FLAGS ${RSL_DEPEND_FLAGS} -I${_dep})
    endforeach()

    # debug
    if(_rslshaders_debug)
        message(STATUS "  RSL_COMPILER: ${RSL_COMPILER}")
        message(STATUS "  RSL_OUTPUT_EXTENSION: ${RSL_OUTPUT_EXTENSION}")
        message(STATUS "  RSL_TARGET: ${RSL_TARGET}")
        message(STATUS "  RSL_SOURCES: ${RSL_SOURCES}")
        message(STATUS "  RSL_COMPILE_FLAGS: ${RSL_COMPILE_FLAGS}")
        message(STATUS "  RSL_DEPENDS: ${RSL_USER_DEPENDS}")
        message(STATUS "  RSL_DEPEND_FLAGS: ${RSL_DEPEND_FLAGS}")
        message(STATUS "  RSL_USER_STAGE_DESTINATION: ${RSL_USER_STAGE_DESTINATION}")
        message(STATUS "  RSL_USER_DESTINATION: ${RSL_USER_DESTINATION}")
    endif(_rslshaders_debug)

    # work out the where to build/stage the shaders
    set(RSL_STAGE_PREFIX ${RSL_STAGE_DESTINATION})
    if(RSL_USER_STAGE_DESTINATION)
        set(RSL_STAGE_PREFIX ${RSL_USER_STAGE_DESTINATION})
    endif(RSL_USER_STAGE_DESTINATION)
    file(MAKE_DIRECTORY ${RSL_STAGE_PREFIX})

    # setup commands to compile each shader
    set(${RSL_TARGET}_OUTPUT)
    foreach(RSL_SOURCE ${RSL_SOURCES})
        get_filename_component(RSL_BASENAME ${RSL_SOURCE} NAME_WE)
        set(RSL_SHADER "${RSL_STAGE_PREFIX}/${RSL_BASENAME}.${RSL_OUTPUT_EXTENSION}")
        list(APPEND ${RSL_TARGET}_OUTPUT ${RSL_SHADER})
        # Add a command to compile the shader
		if(NOT MINGW)
			add_custom_command(
				OUTPUT ${RSL_SHADER}
				COMMAND ${RSL_COMPILER} ${RSL_COMPILE_FLAGS} ${RSL_DEPEND_FLAGS}
					-o ${RSL_SHADER} ${RSL_SOURCE}
				DEPENDS ${RSL_SHADER_DEPENDS} 
				COMMENT "Compiling RSL shader ${RSL_SHADER}"
			)
		else()
			file(TO_NATIVE_PATH "${aqsis_util_location}" aqsis_util_path)
			file(TO_NATIVE_PATH "${aqsis_slcomp_location}" aqsis_slcomp_path)
      set(shared_lib_path "${Boost_LIBRARY_DIRS}")
			get_target_property(aqsl_command ${RSL_COMPILER} LOCATION)
			add_custom_command(
				OUTPUT ${RSL_SHADER}
				COMMAND ${CMAKE_COMMAND} -DRSL_COMPILER="${aqsl_command}"
										 -DRSL_SHADER="${RSL_SHADER}" 
										 -DRSL_COMPILE_FLAGS="${RSL_COMPILE_FLAGS}"
										 -DRSL_DEPEND_FLAGS="${RSL_DEPEND_FLAGS}"
										 -DRSL_SOURCE="${RSL_SOURCE}"
										 -Dutil_path="${aqsis_util_path}"
										 -Dslcomp_path="${aqsis_slcomp_path}"
										 -Dshared_lib_path="${shared_lib_path}"
										 -P ${CMAKE_SOURCE_DIR}/cmake/aqslcompile.cmake
				DEPENDS ${RSL_SHADER_DEPENDS}
				COMMENT "Compiling RSL shader ${RSL_SHADER}"
			)
		endif()
        if(RSL_SETUP_VERBOSE)
          message(STATUS "  ${RSL_SOURCE}")
        endif(RSL_SETUP_VERBOSE)
    endforeach()

    # Add a target which depends on all compiled shaders so that they'll be built
    # prior to the install stage.
    add_custom_target(${RSL_TARGET} ALL DEPENDS ${${RSL_TARGET}_OUTPUT})

ENDMACRO(add_rslshaders)
