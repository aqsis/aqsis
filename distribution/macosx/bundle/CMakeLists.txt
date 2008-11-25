PROJECT(OSXBUNDLE)

SET(BUNDLEDIR "${CMAKE_BINARY_DIR}/bundle"
	CACHE STRING "Staging location for bundles (relative to CMAKE_BINARY_DIR)"
	)
SET(CMAKE_PROJECT_NAME "${AQSIS_PROJECT_NAME_SHORT}"
	CACHE STRING "Package name for OS X"
	)
SET(AQSIS_PACKAGE_NAME "${CMAKE_PROJECT_NAME}-${MAJOR}.${MINOR}.${BUILD}-${CMAKE_SYSTEM_NAME}.dmg"
	CACHE STRING "Package filename for OS X"
	)

SET(DEFAULT_SHADERPATH "./Contents/Resources/shaders")
SET(DEFAULT_ARCHIVEPATH "./Contents/Resources")
SET(DEFAULT_TEXTUREPATH "./Contents/Resources")
SET(DEFAULT_DISPLAYPATH "./Contents/Resources/lib")
SET(DEFAULT_PROCEDURALPATH "./Contents/Resources/lib")
SET(DEFAULT_RESOURCEPATH "./Contents/Resources")
SET(SHADERPATH "%AQSISHOME%/Contents/Resources/shaders:%AQSISHOME%/Contents/Resources/lib")
SET(ARCHIVEPATH "%AQSISHOME%/Contents/Resources")
SET(TEXTUREPATH "%AQSISHOME%/Contents/Resources")
SET(DISPLAYPATH "%AQSISHOME%/Contents/Resources/lib")
SET(PROCEDURALPATH "%AQSISHOME%/Contents/Resources")
SET(RESOURCEPATH "%AQSISHOME%/Contents/Resources")

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/aqsisrc.in.cmake ${BUNDLEDIR}/aqsisrc)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/create_bundle.sh.in.cmake ${BUNDLEDIR}/create_bundle.sh)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in.cmake ${BUNDLEDIR}/Info.plist)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/../application.icns ${BUNDLEDIR}/Aqsis.icns COPYONLY)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bundle_config.app
	COMMAND ${AQSIS_OSACOMPILE_EXECUTABLE} -o ${BUNDLEDIR}/bundle_config.app ${CMAKE_CURRENT_SOURCE_DIR}/bundle_config.applescript
	)
ADD_CUSTOM_TARGET(bundleconfig ALL echo
	DEPENDS  ${CMAKE_CURRENT_BINARY_DIR}/bundle_config.app
	)
	
ADD_CUSTOM_TARGET(osxbundle
	WORKING_DIRECTORY ${BUNDLEDIR}
	COMMAND sh ./create_bundle.sh
	)

ADD_DEPENDENCIES(osxbundle aqsis aqsl aqsltell eqsl miqser piqsl teqser display d_bmp d_exr d_xpm piqsldisplay shaders ribparse bundleconfig)
