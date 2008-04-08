PROJECT(OSXBUNDLE)

SET(BUNDLEDIR "${CMAKE_BINARY_DIR}/bundle"
	CACHE STRING "Staging location for bundles (relative to CMAKE_BINARY_DIR)"
	)
SET(CMAKE_PROJECT_NAME "Aqsis"
	CACHE STRING "Bundle Name for Mac OS X"
	)

SET(SHADERPATH ../Resources/shaders)
SET(ARCHIVEPATH "")
SET(TEXTUREPATH "")
SET(DISPLAYPATH ../Resources/lib)
SET(PROCEDURALPATH "")
SET(PLUGINPATH "")
SET(RESOURCEPATH "")

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/aqsisrc.in.cmake ${BUNDLEDIR}/aqsisrc)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/create_bundle.sh.in.cmake ${BUNDLEDIR}/create_bundle.sh)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in.cmake ${BUNDLEDIR}/Info.plist)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/../application.icns ${BUNDLEDIR}/Aqsis.icns COPYONLY)


ADD_CUSTOM_TARGET(osxbundle
	WORKING_DIRECTORY ${BUNDLEDIR}
	COMMAND sh ./create_bundle.sh
	)

ADD_DEPENDENCIES(osxbundle aqsis aqsl aqsltell eqsl miqser piqsl teqser display piqsldisplay shaders)
