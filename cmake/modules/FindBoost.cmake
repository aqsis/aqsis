######################################################################
# Posix specific configuration

IF(UNIX AND NOT APPLE)
	FIND_PATH(BOOST_INCLUDE_DIR boost
		/usr/include
		DOC "Directory where the boost header files are located"
		)

	SET(BOOST_LIB_DIR /usr/lib CACHE PATH "Directory where the boost libraries are located")

	SET(BOOST_THREAD_LIB boost_thread CACHE STRING "")

ENDIF(UNIX AND NOT APPLE)

######################################################################
# Apple specific configuration

IF(UNIX AND APPLE)
	FIND_PATH(BOOST_INCLUDE_DIR boost
		/opt/local/include
		DOC "Directory where the boost header files are located"
		)

	SET(BOOST_LIB_DIR /opt/local/lib CACHE PATH "Directory where the boost libraries are located")

	SET(BOOST_THREAD_LIB boost_thread CACHE STRING "")

ENDIF(UNIX AND APPLE)

######################################################################
# Win32 specific configuration

IF(WIN32)
	FIND_PATH(BOOST_INCLUDE_DIR boost
		c:/boost/include/boost-1_33_1
		DOC "Directory where the boost header files are located"
		)

	SET(BOOST_LIB_DIR c:/boost/lib CACHE PATH "Directory where the boost libraries are located")

        IF(MSVC)
                SET(BOOST_PROGRAM_OPTIONS_LIB optimized libboost_thread-vc80-mt debug libboost_thread-vc80-mt-gd CACHE STRING "")
        ELSE(MSVC)
                SET(BOOST_PROGRAM_OPTIONS_LIB libboost_thread-mgw34-1_34 CACHE STRING "")
        ENDIF(MSVC)

ENDIF(WIN32)

# Following are the consolidated variables that should be used for builds
SET(BOOST_INCLUDE_DIRS
	${BOOST_INCLUDE_DIR}
	)

SET(BOOST_LIB_DIRS
	${BOOST_LIB_DIR}
	)

SET(BOOST_PROGRAM_OPTIONS_LIBS
	${BOOST_THREAD_LIB}
	)

SET(BOOST_FOUND 1)

