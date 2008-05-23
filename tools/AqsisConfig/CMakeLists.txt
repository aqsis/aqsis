PROJECT(AQSISCONFIG)

FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/AqsisConfig.xcodeproj 
	${CMAKE_CURRENT_BINARY_DIR}/English.lproj
	${CMAKE_CURRENT_BINARY_DIR}/English.lproj/MainMenu.nib
	)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/AqsisConfig.applescript 
	${CMAKE_CURRENT_BINARY_DIR}/AqsisConfig.applescript COPYONLY)	
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/AqsisConfig.xcodeproj/project.pbxproj
	${CMAKE_CURRENT_BINARY_DIR}/AqsisConfig.xcodeproj/project.pbxproj COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
	${CMAKE_CURRENT_BINARY_DIR}/Info.plist COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/English.lproj/MainMenu.nib/classes.nib
	${CMAKE_CURRENT_BINARY_DIR}/English.lproj/MainMenu.nib/classes.nib COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/English.lproj/MainMenu.nib/info.nib
	${CMAKE_CURRENT_BINARY_DIR}/English.lproj/MainMenu.nib/info.nib COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/English.lproj/MainMenu.nib/keyedobjects.nib
	${CMAKE_CURRENT_BINARY_DIR}/English.lproj/MainMenu.nib/keyedobjects.nib COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/English.lproj/MainMenu.nib/data.dependency
	${CMAKE_CURRENT_BINARY_DIR}/English.lproj/MainMenu.nib/data.dependency COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/English.lproj/InfoPlist.strings
	${CMAKE_CURRENT_BINARY_DIR}/English.lproj/InfoPlist.strings COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/main.m
	${CMAKE_CURRENT_BINARY_DIR}/main.m COPYONLY)
	
ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Release/AqsisConfig.app
	COMMAND ${AQSIS_XCODEBUILD_EXECUTABLE} SYMROOT=${CMAKE_CURRENT_BINARY_DIR} 
	)
ADD_CUSTOM_TARGET(aqsisconfigurator ALL echo
	DEPENDS  ${CMAKE_CURRENT_BINARY_DIR}/Release/AqsisConfig.app
	)

