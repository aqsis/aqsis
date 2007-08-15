SET(OPENEXR_FOUND 0)

######################################################################
# Posix specific configuration

IF(UNIX AND NOT APPLE)
	INCLUDE(FindPkgConfig)
	PKG_CHECK_MODULES(OPENEXR OpenEXR)

	IF(OPENEXR_FOUND)
		SET(AQSIS_OPENEXR_INCLUDE_DIR
			${OPENEXR_INCLUDE_DIRS}
			CACHE PATH "Location of OpenEXR includes"
			)

		SET(AQSIS_OPENEXR_LIB_DIR
			${OPENEXR_LIBRARY_DIRS}
			CACHE PATH "Location of OpenEXR libraries"
			)

		SET(AQSIS_OPENEXR_LIBS
			${OPENEXR_LIBRARIES}
			CACHE PATH "Libraries to link for OpenEXR"
			)

	ENDIF(OPENEXR_FOUND)

ENDIF(UNIX AND NOT APPLE)

######################################################################
# Win32 specific configuration

IF(WIN32)
	SET(AQSIS_OPENEXR_INCLUDE_DIRS NOT_FOUND
		CACHE PATH "Location of OpenEXR includes"
		)
	SET(AQSIS_OPENEXR_LIB_DIRS NOT_FOUND
		CACHE PATH "Location of OpenEXR libraries"
		)
	SET(AQSIS_OPENEXR_LIBS NOT_FOUND
		CACHE PATH "Libraries to link for OpenEXR"
		)
ENDIF(WIN32)

######################################################################
# Mac OS X specific configuration

IF(APPLE)
	SET(AQSIS_OPENEXR_INCLUDE_DIRS NOT_FOUND
		CACHE PATH "Location of OpenEXR includes"
		)
	SET(AQSIS_OPENEXR_LIB_DIRS NOT_FOUND
		CACHE PATH "Location of OpenEXR libraries"
		)
	SET(AQSIS_OPENEXR_LIBS NOT_FOUND
		CACHE PATH "Libraries to link for OpenEXR"
		)
ENDIF(APPLE)

