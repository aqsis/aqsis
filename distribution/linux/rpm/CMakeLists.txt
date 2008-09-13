PROJECT(SPECPACKAGE)

SET(SPECPACKAGEDIR "${CMAKE_BINARY_DIR}/specpackage"
	CACHE STRING "Staging location for Linux SPEC files (relative to CMAKE_BINARY_DIR)"
	)

SET(CMAKE_PROJECT_NAME "${AQSIS_PROJECT_NAME_BASIC}"
	CACHE STRING "Package name for Linux"
	)

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/aqsis.spec.in.cmake" "${SPECPACKAGEDIR}/aqsis.spec")

ADD_CUSTOM_TARGET(specpackage
	COMMAND echo "SPEC file(s) created within ${SPECPACKAGEDIR}"
	)
