PROJECT(AQSISTYPES)

#----------------------------------------------------------------------
# create aqsis_config.h
INCLUDE (CheckIncludeFiles)

CHECK_INCLUDE_FILES(stdint.h AQSIS_HAVE_STDINT_H)
CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/aqsis_config.h.in.cmake"
	"${PROJECT_BINARY_DIR}/aqsis_config.h")


#----------------------------------------------------------------------
# Source files

SET(AQSISTYPES_SRCS
	bitvector.cpp
	cellnoise.cpp
	color.cpp
	exception.cpp
	file.cpp
	logging.cpp
	lowdiscrep.cpp
	matrix.cpp
	memorysentry.cpp
	noise.cpp
	noise1234.cpp
	plugins.cpp
	popen.cpp
	primvartoken.cpp
	random.cpp
	refcount.cpp
	riparamlist.cpp
	spline.cpp
	sstring.cpp
	tokendictionary.cpp
)

SET(AQSISTYPES_HDRS
	aqsis.h
	aqsis_types.h
	aqsismath.h
	autobuffer.h
	bitvector.h
	cellnoise.h
	color.h
	enum.h
	exception.h
	execute.h
	file.h
	interpclasscounts.h
	list.h
	logging.h
	logging_streambufs.h
	lowdiscrep.h
	matrix.h
	matrix2d.h
	memorysentry.h
	multitimer.h
	noise.h
	noise1234.h
	plugins.h
	pool.h
	popen.h
	primvartoken.h
	primvartype.h
	random.h
	refcount.h
	region.h
	ri_convenience.h
	ri_types.h
	riparamlist.h
	smartptr.h
	socket.h
	spline.h
	sstring.h
	tokendictionary.h
	vector2d.h
	vector3d.h
	vector4d.h
	vectorcast.h
)

IF(UNIX OR APPLE)
	SET(AQSISTYPES_SYSTEM_SRCS
		posix/execute_system.cpp
		posix/socket_system.cpp
	)
	SET(AQSISTYPES_SYSTEM_HDRS
		posix/aqsis_compiler.h
		posix/socket_system.h
	)
ELSE(UNIX OR APPLE)
	IF(WIN32)
		SET(AQSISTYPES_SYSTEM_SRCS
			win32/execute_system.cpp
			win32/socket_system.cpp
		)
		SET(AQSISTYPES_SYSTEM_HDRS
			win32/aqsis_compiler.h
			win32/socket_system.h
			win32/unistd.h
		)
	ENDIF(WIN32)
ENDIF(UNIX OR APPLE)

SET(AQSISTYPES_SRCS ${AQSISTYPES_SRCS} ${AQSISTYPES_SYSTEM_SRCS})
SET(AQSISTYPES_HDRS
	${AQSISTYPES_HDRS}
	${AQSISTYPES_SYSTEM_HDRS}
	"${PROJECT_BINARY_DIR}/aqsis_config.h"
)

#------------------------------------------------------------------------------
# Targets
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

# Internal dependencies
INCLUDE(${PROJECT_SOURCE_DIR}/export.cmake)

# Group header files for Visual Studio
SOURCE_GROUP("Header files" FILES ${AQSISTYPES_HDRS})

ADD_LIBRARY(aqsistypes SHARED ${AQSISTYPES_SRCS} ${AQSISTYPES_HDRS} ${INFORES_SRCS})

# Add the export definition using target properties to 
# avoid it propagating down to the test targets.
SET_TARGET_PROPERTIES(aqsistypes PROPERTIES COMPILE_DEFINITIONS COMMON_EXPORTS)

GET_TARGET_PROPERTY(AQSISTYPES_LIBRARY aqsistypes LOCATION)

SET(LINKLIBS
	${AQSISTYPES_SYSTEM_LINKLIBS}
	${Boost_FILESYSTEM_LIBRARY}
)
TARGET_LINK_LIBRARIES(aqsistypes ${LINKLIBS})

SET_TARGET_PROPERTIES(aqsistypes PROPERTIES OUTPUT_NAME "aqsis_types" SOVERSION ${MAJOR} VERSION "${MAJOR}.${MINOR}")

INSTALL(TARGETS aqsistypes
	RUNTIME DESTINATION ${BINDIR}
	LIBRARY DESTINATION ${LIBDIR}
	ARCHIVE DESTINATION ${LIBDIR})

INSTALL(FILES ${AQSISTYPES_HDRS} DESTINATION ${INCLUDEDIR})

#----------------------------------------------------------------------
# Testing
SET(AQSISTYPES_TEST_SRCS
	aqsismath_test.cpp
	cellnoise_test.cpp
	color_test.cpp
	enum_test.cpp
	file_test.cpp
	matrix2d_test.cpp
	matrix_test.cpp
	noise1234_test.cpp
	noise_test.cpp
	popen_test.cpp
	primvartoken_test.cpp
	spline_test.cpp
	vector2d_test.cpp
	vector3d_test.cpp
	vector4d_test.cpp
)

SET(LINKLIBS)
INCLUDE(link.cmake)
AQSIS_ADD_TESTS(${AQSISTYPES_TEST_SRCS} LINKWITH ${LINKLIBS})

IF(AQSIS_ENABLE_TESTING)
	# A helper executable for popen_test.cpp
	ADD_EXECUTABLE(pipethrough popen_pipethrough_helper_test.cpp)
	SET_TARGET_PROPERTIES(pipethrough PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
	)
	ADD_DEPENDENCIES(popen_test pipethrough)
ENDIF(AQSIS_ENABLE_TESTING)
