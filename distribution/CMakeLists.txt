# Set some variables used in configuring various packaging stuff.
SET(AQSIS_PROJECT_NAME "Aqsis Renderer")
SET(AQSIS_PROJECT_NAME_SHORT "Aqsis")
SET(AQSIS_PROJECT_NAME_BASIC "aqsis")
SET(AQSIS_PROJECT_VENDOR "Aqsis Team")
SET(AQSIS_PROJECT_COPYRIGHT "Copyright (C) 2009, ${AQSIS_PROJECT_VENDOR}.")
SET(AQSIS_PROJECT_COPYRIGHT_OTHER "The RenderMan(R) Interface Procedures and Protocol are Copyright 1988, 1989, 2000, 2005 Pixar. All Rights Reserved.")

if(WIN32)
	# Temporary resource file hack as it seems CMake has issues when using the
	# 'configure_file()' command within an 'include()' file, like 'windows.cmake'.
	configure_file("${CMAKE_SOURCE_DIR}/distribution/win/info.rc.in.cmake" info.rc)
	configure_file("${CMAKE_SOURCE_DIR}/distribution/win/icon.rc.in.cmake" icon.rc)
	set(INFORES_SRCS info.rc)
	set(ICONRES_SRCS icon.rc)

	# There is a bug in NSI that does not handle full unix paths properly. Make
	# sure there is at least one set of four (4) backlasshes.
	set(CPACK_PACKAGE_FILE_NAME "${AQSIS_PROJECT_NAME_BASIC}-setup-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}")
	set(CPACK_PACKAGE_EXECUTABLES "eqsl;${AQSIS_PROJECT_NAME}")
	set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/distribution/win\\\\header.bmp")
	set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${AQSIS_PROJECT_NAME_SHORT} ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}")
	set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\eqsl.exe")
	set(CPACK_NSIS_DISPLAY_NAME "${AQSIS_PROJECT_NAME} ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}")
	set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\community.aqsis.org")
	set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.aqsis.org")
	set(CPACK_NSIS_CONTACT "packages@aqsis.org")
	set(CPACK_NSIS_MODIFY_PATH ON)
	#set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
	#	!include \\\"TextFunc.nsh\\\"
	#	!include \\\"StrFunc.nsh\\\"
	#	${StrRep}
	#	!insertmacro ConfigWrite
	#	${StrRep} $R0 $INSTDIR \\\"\\\\\\\" \\\"/\\\"
	#	${ConfigWrite} \\\"$INSTDIR\\\\bin\\\\aqsisrc\\\" 'Option \\\"defaultsearchpath\\\" \\\"string shader\\\" ' '[\\\"$R0/shaders/displacement:$R0/shaders/imager:$R0/shaders/light:$R0/shaders/surface:$R0/shaders/volume\\\"]' $R1
	#	${ConfigWrite} \\\"$INSTDIR\\\\bin\\\\aqsisrc\\\" 'Option \\\"defaultsearchpath\\\" \\\"string archive\\\" ' '[\\\"$R0\\\"]' $R2
	#	${ConfigWrite} \\\"$INSTDIR\\\\bin\\\\aqsisrc\\\" 'Option \\\"defaultsearchpath\\\" \\\"string texture\\\" ' '[\\\"$R0\\\"]' $R3
	#	${ConfigWrite} \\\"$INSTDIR\\\\bin\\\\aqsisrc\\\" 'Option \\\"defaultsearchpath\\\" \\\"string display\\\" ' '[\\\"$R0/bin\\\"]' $R4
	#	${ConfigWrite} \\\"$INSTDIR\\\\bin\\\\aqsisrc\\\" 'Option \\\"defaultsearchpath\\\" \\\"string procedural\\\" ' '[\\\"$R0/bin\\\"]' $R5
	#	${ConfigWrite} \\\"$INSTDIR\\\\bin\\\\aqsisrc\\\" 'Option \\\"defaultsearchpath\\\" \\\"string resource\\\" ' '[\\\"$R0\\\"]' $R6
	#	")
	set(CPACK_ALL_INSTALL_TYPES Full Minimal)
	set(CPACK_COMPONENT_MAIN_INSTALL_TYPES Full Minimal)
	set(CPACK_COMPONENT_DEVELOPMENT_INSTALL_TYPES Full)
	set(CPACK_COMPONENT_EXAMPLES_INSTALL_TYPES Full)
	set(CPACK_COMPONENT_SHADERS_INSTALL_TYPES Full)
	set(CPACK_GENERATOR "NSIS")
	set(CPACK_SOURCE_GENERATOR "ZIP")
	# Cusotm Windows NSIS installer
	#add_subdirectory(win/nsis)

elseif(APPLE)
	set(CPACK_PACKAGE_FILE_NAME
		"${AQSIS_PROJECT_NAME_SHORT}-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}-${CMAKE_SYSTEM_NAME}")
	#set(CPACK_PACKAGE_ICON "")
	set(CPACK_BUNDLE_NAME "${AQSIS_PROJECT_NAME_SHORT}")
	set(CPACK_BUNDLE_ICON "${BUNDLEDIR}/Aqsis.icns")
	set(CPACK_BUNDLE_PLIST "${BUNDLEDIR}/Info.plist")
	set(CPACK_BUNDLE_STARTUP_COMMAND "")
	set(CPACK_GENERATOR "Bundle")
	set(CPACK_SOURCE_GENERATOR "TGZ")
	# Custom OS X Bundle
	add_subdirectory(macosx/bundle)

elseif(UNIX)
	set(CPACK_SOURCE_PACKAGE_FILE_NAME
		"${AQSIS_PROJECT_NAME_BASIC}-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}")
	set(CPACK_GENERATOR "TGZ")
	set(CPACK_SOURCE_GENERATOR "TGZ")
	# Cusotm Linux RPM
	#add_subdirectory(linux/rpm)

endif()


#--------------------------------------------------
# Generic Packaging setup
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "${AQSIS_PROJECT_NAME_SHORT}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${AQSIS_PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "${AQSIS_PROJECT_VENDOR}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/ChangeLog.txt")
set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_SOURCE_DIR}/README")
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_BUILD}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${AQSIS_PROJECT_NAME_SHORT}")

set(CPACK_COMPONENTS_ALL main examples shaders development Unspecified)
set(CPACK_COMPONENT_MAIN_REQUIRED TRUE)
set(CPACK_COMPONENT_UNSPECIFIED_REQUIRED TRUE)
set(CPACK_COMPONENT_UNSPECIFIED_HIDDEN TRUE)
set(CPACK_COMPONENT_EXAMPLES_GROUP "content")
set(CPACK_COMPONENT_SHADERS_GROUP "content")
set(CPACK_COMPONENT_MAIN_DESCRIPTION
	"Rendering application and essential files only")
set(CPACK_COMPONENT_GROUP_CONTENT_DESCRIPTION
	"Examples and shader source files")
set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION
	"Example files (.rib)")
set(CPACK_COMPONENT_SHADERS_DESCRIPTION
	"Generic shader source files (.sl)")
set(CPACK_COMPONENT_DEVELOPMENT_DESCRIPTION
	"Include and library files for software development")

include(CPack)
