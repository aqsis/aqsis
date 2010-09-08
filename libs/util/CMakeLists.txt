project(util)

# Check for boost filesystem.
if(NOT Boost_FILESYSTEM_FOUND)
	message(FATAL_ERROR "Aqsis util requires boost filesystem to build")
endif()

set(util_srcs
	argparse.cpp
	bitvector.cpp
	exception.cpp
	file.cpp
	logging.cpp
	plugins.cpp
	popen.cpp
	sstring.cpp
)
if(UNIX)
	set(util_srcs
		${util_srcs}
		posix/execute_system.cpp
		posix/socket_system.cpp
	)
elseif(WIN32)
	set(util_srcs
		${util_srcs}
		win32/execute_system.cpp
		win32/socket_system.cpp
	)
endif()

set(util_test_srcs
	enum_test.cpp
	file_test.cpp
)
#argparse_test.cpp  # <-- TODO: make into a unit test

set(linklibs ${Boost_FILESYSTEM_LIBRARY})
if(UNIX)
	list(APPEND linklibs dl)
elseif(WIN32)
	list(APPEND linklibs ws2_32)
endif()
if(Boost_SYSTEM_FOUND)
	list(APPEND linklibs ${Boost_SYSTEM_LIBRARY})
endif()

aqsis_add_library(aqsis_util ${util_srcs} ${util_hdrs}
	TEST_SOURCES ${util_test_srcs}
	COMPILE_DEFINITIONS AQSIS_UTIL_EXPORTS
	DEPENDS 
	LINK_LIBRARIES ${linklibs}
)

aqsis_install_targets(aqsis_util)

if(aqsis_enable_testing)
	# Special case for popen testing...  This is slightly painful to embed in
	# the library so it hasn't been done as yet.
	add_executable(popen_test popen_test.cpp)
	target_link_libraries(popen_test aqsis_util)
	set_target_properties(popen_test PROPERTIES
		COMPILE_DEFINITIONS "BOOST_TEST_MODULE=popen_tests")
	add_test(popen_test popen_test)
	# A helper executable for popen_test.cpp
	add_executable(pipethrough popen_pipethrough_helper_test.cpp)
	add_dependencies(popen_test pipethrough)
endif()
