# Disallow in-source build
STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" AQSIS_IN_SOURCE)
IF(AQSIS_IN_SOURCE)
	MESSAGE(FATAL_ERROR "Aqsis requires an out of source build. Please create a separate build directory and run 'cmake path_to_source [options]' there.")
ENDIF(AQSIS_IN_SOURCE)

