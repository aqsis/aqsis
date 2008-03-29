PROJECT(OSXBUNDLE)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/create_bundle.sh.in.cmake 
	${BUNDLEDIR}/create_bundle.sh)	
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in.cmake 
	${BUNDLEDIR}/Info.plist)	
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Aqsis.icns 
	${BUNDLEDIR}/Aqsis.icns COPYONLY)	

ADD_CUSTOM_TARGET(osxbundle
	WORKING_DIRECTORY ${BUNDLEDIR}
	COMMAND sh ./create_bundle.sh
	)

ADD_DEPENDENCIES(osxbundle shaders apiheader aqsisconfigurator)

