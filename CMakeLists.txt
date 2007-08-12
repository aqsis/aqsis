PROJECT(aqsis_all)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

INCLUDE(FindTIFF)
INCLUDE(FindBoost)
INCLUDE(FindZLIB)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# TODO: This should probably be in a separate macros file.
MACRO(FILTER_OUT FILTERS INPUTS OUTPUT)
   # Mimicks Gnu Make's $(filter-out) which removes elements 
   # from a list that match the pattern.
   # Arguments:
   #  FILTERS - list of patterns that need to be removed
   #  INPUTS  - list of inputs that will be worked on
   #  OUTPUT  - the filtered list to be returned
   # 
   # Example: 
   #  SET(MYLIST this that and the other)
   #  SET(FILTS this that)
   #
   #  FILTER_OUT("${FILTS}" "${MYLIST}" OUT)
   #  MESSAGE("OUTPUT = ${OUT}")
   #
   # The output - 
   #   OUTPUT = and;the;other
   #
   SET(FOUT "")
   FOREACH(INP ${INPUTS})
	   SET(FILTERED 0)
	   FOREACH(FILT ${FILTERS})
		   IF(${FILTERED} EQUAL 0)
			   IF("${FILT}" STREQUAL "${INP}")
				   SET(FILTERED 1)
			   ENDIF("${FILT}" STREQUAL "${INP}")
		   ENDIF(${FILTERED} EQUAL 0)
	   ENDFOREACH(FILT ${FILTERS})
	   IF(${FILTERED} EQUAL 0)
		   SET(FOUT ${FOUT} ${INP})
	   ENDIF(${FILTERED} EQUAL 0)
   ENDFOREACH(INP ${INPUTS})
   SET(${OUTPUT} ${FOUT})
ENDMACRO(FILTER_OUT FILTERS INPUTS OUTPUT)

# TODO: find these properly
SET(FLEX_EXECUTABLE flex)
SET(BISON_EXECUTABLE bison)

IF(WIN32)
	ADD_DEFINITIONS(-DNO_SYSLOG)
ELSE(WIN32)
	ADD_DEFINITIONS(-DDEFAULT_PLUGIN_PATH="\\"/usr/local/aqsis/lib/aqsis/plugins\\"")
	ADD_DEFINITIONS(-DDEFAULT_RC_PATH="\\"/usr/local/aqsis/lib/aqsis/plugins\\"")
ENDIF(WIN32)

SET(MAJOR 1)
SET(MINOR 3)
SET(BUILD 0)
CONFIGURE_FILE(version.h.in.cmake ${PROJECT_BINARY_DIR}/version.h)

INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

SUBDIRS(argparse)
SUBDIRS(rib/api)
SUBDIRS(rib/rib2)
SUBDIRS(rib/rib2ri)
SUBDIRS(renderer/ddmanager)
SUBDIRS(renderer/raytrace)
SUBDIRS(shadercompiler/shaderexecenv)
SUBDIRS(aqsistypes)
SUBDIRS(shadercompiler/shadervm)
SUBDIRS(renderer/render)
SUBDIRS(renderer/aqsis)
SUBDIRS(shadercompiler/slpp)
SUBDIRS(shadercompiler/slparse)
SUBDIRS(shadercompiler/codegenvm)
SUBDIRS(shadercompiler/aqsl)
SUBDIRS(shadercompiler/slxargs)
SUBDIRS(shadercompiler/aqsltell)

