SET(AQSIS_BOOST_FOUND 0)

SET(AQSIS_BOOST_VERSION_SUFFIX "-1_34_1" CACHE STRING "Version suffix used on boost libraries (including '-' prefix).")

SET(AQSIS_BOOST_LIBRARIES_DIR "" CACHE PATH "Location to look for boost libraries.")

SET(AQSIS_BOOST_INCLUDE_SEARCHPATH)

IF(WIN32)
	IF(MSVC AND MSVC80)
		SET(AQSIS_BOOST_THREAD_LIBRARY_NAME boost_thread-vc80-mt${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_thread library (undecorated, i.e. the string passed to the compiler)")
		SET(AQSIS_BOOST_UNIT_TEST_FRAMEWORK_LIBRARY_NAME boost_unit_test_framework-vc80-mt${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_unit_test_framework library (undecorated, i.e. the string passed to the compiler)")
		SET(AQSIS_BOOST_WAVE_LIBRARY_NAME boost_wave-vc80-mt${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_wave library (undecorated, i.e. the string passed to the compiler)")
		SET(AQSIS_BOOST_FILESYSTEM_LIBRARY_NAME boost_filesystem-vc80-mt${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_filesystem library (undecorated, i.e. the string passed to the compiler)")
	ELSEIF(MSVC AND MSVC90)
		SET(AQSIS_BOOST_THREAD_LIBRARY_NAME libboost_thread-vc90-mt${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_thread library (undecorated, i.e. the string passed to the compiler)")
		SET(AQSIS_BOOST_UNIT_TEST_FRAMEWORK_LIBRARY_NAME libboost_unit_test_framework-vc90-mt${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_unit_test_framework library (undecorated, i.e. the string passed to the compiler)")
		SET(AQSIS_BOOST_WAVE_LIBRARY_NAME libboost_wave-vc90-mt${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_wave library (undecorated, i.e. the string passed to the compiler)")
		SET(AQSIS_BOOST_FILESYSTEM_LIBRARY_NAME libboost_filesystem-vc90-mt${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_filesystem library (undecorated, i.e. the string passed to the compiler)")
	ELSE(MSVC AND MSVC80)
		SET(AQSIS_BOOST_THREAD_LIBRARY_NAME libboost_thread-mgw34${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_thread library (undecorated, i.e. the string passed to the compiler)")
		SET(AQSIS_BOOST_UNIT_TEST_FRAMEWORK_LIBRARY_NAME libboost_unit_test_framework-mgw34${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_unit_test_framework library (undecorated, i.e. the string passed to the compiler)")
		SET(AQSIS_BOOST_WAVE_LIBRARY_NAME libboost_wave-mgw34${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_wave library (undecorated, i.e. the string passed to the compiler)")
		SET(AQSIS_BOOST_FILESYSTEM_LIBRARY_NAME libboost_filesystem-mgw34${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_filesystem library (undecorated, i.e. the string passed to the compiler)")
	ENDIF(MSVC AND MSVC80)
ELSE(WIN32)
	IF(APPLE)
		SET(AQSIS_BOOST_THREAD_LIBRARY_NAME boost_thread-mt${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_thread library (undecorated, i.e. the string passed to the compiler)")
		SET(AQSIS_BOOST_UNIT_TEST_FRAMEWORK_LIBRARY_NAME boost_unit_test_framework-mt${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_unit_test_framework library (undecorated, i.e. the string passed to the compiler)")
		SET(AQSIS_BOOST_WAVE_LIBRARY_NAME boost_wave-mt${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_wave library (undecorated, i.e. the string passed to the compiler)")
		SET(AQSIS_BOOST_FILESYSTEM_LIBRARY_NAME boost_filesystem-mt${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_filesystem library (undecorated, i.e. the string passed to the compiler)")
	ENDIF(APPLE)
	SET(AQSIS_BOOST_THREAD_LIBRARY_NAME boost_thread-gcc-mt${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_thread library (undecorated, i.e. the string passed to the compiler)")
	SET(AQSIS_BOOST_UNIT_TEST_FRAMEWORK_LIBRARY_NAME boost_unit_test_framework-gcc-mt${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_unit_test_framework library (undecorated, i.e. the string passed to the compiler)")
	SET(AQSIS_BOOST_WAVE_LIBRARY_NAME boost_wave-gcc-mt${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_wave library (undecorated, i.e. the string passed to the compiler)")
	SET(AQSIS_BOOST_FILESYSTEM_LIBRARY_NAME boost_filesystem-gcc-mt${AQSIS_BOOST_VERSION_SUFFIX} CACHE STRING "The name of the boost_filesystem library (undecorated, i.e. the string passed to the compiler)")
ENDIF(WIN32)
MARK_AS_ADVANCED(AQSIS_BOOST_VERSION_SUFFIX)
MARK_AS_ADVANCED(AQSIS_BOOST_THREAD_LIBRARY_NAME)
MARK_AS_ADVANCED(AQSIS_BOOST_UNIT_TEST_FRAMEWORK_LIBRARY_NAME)
MARK_AS_ADVANCED(AQSIS_BOOST_WAVE_LIBRARY_NAME)
MARK_AS_ADVANCED(AQSIS_BOOST_FILESYSTEM_LIBRARY_NAME)

FIND_PATH(AQSIS_BOOST_INCLUDE_DIR
			boost
			PATHS ${AQSIS_BOOST_INCLUDE_SEARCHPATH}
			DOC "Location of the boost headers folder"
			)

FIND_LIBRARY(AQSIS_BOOST_THREAD_LIBRARY
			NAMES ${AQSIS_BOOST_THREAD_LIBRARY_NAME}	
			PATHS ${AQSIS_BOOST_LIBRARIES_DIR}
			DOC "Location of the boost thread library"
			)
MARK_AS_ADVANCED(AQSIS_BOOST_THREAD_LIBRARY)

FIND_LIBRARY(AQSIS_BOOST_UNIT_TEST_FRAMEWORK_LIBRARY
			NAMES ${AQSIS_BOOST_UNIT_TEST_FRAMEWORK_LIBRARY_NAME}	
			PATHS ${AQSIS_BOOST_LIBRARIES_DIR}
			DOC "Location of the boost unit test framework library"
			)
MARK_AS_ADVANCED(AQSIS_BOOST_UNIT_TEST_FRAMEWORK_LIBRARY)

FIND_LIBRARY(AQSIS_BOOST_WAVE_LIBRARY
			NAMES ${AQSIS_BOOST_WAVE_LIBRARY_NAME}	
			PATHS ${AQSIS_BOOST_LIBRARIES_DIR}
			DOC "Location of the boost wave library"
			)
MARK_AS_ADVANCED(AQSIS_BOOST_WAVE_LIBRARY)

FIND_LIBRARY(AQSIS_BOOST_FILESYSTEM_LIBRARY
			NAMES ${AQSIS_BOOST_FILESYSTEM_LIBRARY_NAME}	
			PATHS ${AQSIS_BOOST_LIBRARIES_DIR}
			DOC "Location of the boost filesystem library"
			)
MARK_AS_ADVANCED(AQSIS_BOOST_FILESYSTEM_LIBRARY)

STRING(COMPARE EQUAL "${AQSIS_BOOST_INCLUDE_DIR}" "AQSIS_BOOST_INCLUDE_DIR-NOTFOUND" AQSIS_BOOST_INCLUDE_NOTFOUND)
STRING(COMPARE EQUAL "${AQSIS_BOOST_THREAD_LIBRARY}" "AQSIS_BOOST_THREAD_LIBRARY-NOTFOUND" AQSIS_BOOST_THREAD_LIBRARY_NOTFOUND)
STRING(COMPARE EQUAL "${AQSIS_BOOST_WAVE_LIBRARY}" "AQSIS_BOOST_WAVE_LIBRARY-NOTFOUND" AQSIS_BOOST_WAVE_LIBRARY_NOTFOUND)

IF(NOT AQSIS_BOOST_THREAD_LIBRARY_NOTFOUND AND NOT AQSIS_BOOST_WAVE_LIBRARY_NOTFOUND AND NOT AQSIS_BOOST_FILESYSTEM_LIBRARY_NOTFOUND AND NOT AQSIS_BOOST_INCLUDE_DIR_NOTFOUND)
	SET(AQSIS_BOOST_FOUND 1)
ENDIF(NOT AQSIS_BOOST_THREAD_LIBRARY_NOTFOUND AND NOT AQSIS_BOOST_WAVE_LIBRARY_NOTFOUND AND NOT AQSIS_BOOST_FILESYSTEM_LIBRARY_NOTFOUND AND NOT AQSIS_BOOST_INCLUDE_DIR_NOTFOUND)

