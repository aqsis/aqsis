PROJECT(OSXBUNDLE)

SET(SHADERPATH ../shaders)
SET(ARCHIVEPATH "")
SET(TEXTUREPATH "")
SET(DISPLAYPATH ../lib)
SET(PROCEDURALPATH "")
SET(PLUGINPATH "")
SET(RESOURCEPATH "")

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/aqsisrc.in.cmake 
	${BUNDLEDIR}/aqsisrc)	

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/create_bundle.sh.in.cmake 
	${BUNDLEDIR}/create_bundle.sh)	
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in.cmake 
	${BUNDLEDIR}/Info.plist)	

ADD_CUSTOM_TARGET(osxbundle
	WORKING_DIRECTORY ${BUNDLEDIR}
	COMMAND sh ./create_bundle.sh
	)

ADD_DEPENDENCIES(osxbundle aqsis piqsl eqsl aqsl aqsltell miqser shaders apiheader aqsisconfigurator)

