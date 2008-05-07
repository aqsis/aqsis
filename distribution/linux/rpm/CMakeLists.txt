PROJECT(SPECPACKAGE)

SET(SPECPACKAGEDIR "${CMAKE_BINARY_DIR}/specpackage"
	CACHE STRING "Staging location for Linux SPEC files (relative to CMAKE_BINARY_DIR)"
	)

SET(CMAKE_PROJECT_NAME "aqsis"
	CACHE STRING "Package name for Linux"
	)

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/aqsis-Fedora_8.spec.in.cmake" "${SPECPACKAGEDIR}/aqsis-Fedora_8.spec")

ADD_CUSTOM_TARGET(specpackage
	COMMAND echo "SPEC file(s) created within ${SPECPACKAGEDIR}"
	)
