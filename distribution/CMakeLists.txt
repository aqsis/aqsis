# Set some variables used in configuring various packaging stuff.
SET(AQSIS_PROJECT_NAME "Aqsis Renderer")
SET(AQSIS_PROJECT_NAME_SHORT "Aqsis")
SET(AQSIS_PROJECT_NAME_BASIC "aqsis")
SET(AQSIS_PROJECT_VENDOR "Aqsis Team")
SET(AQSIS_PROJECT_COPYRIGHT "Copyright (C) 2009, ${AQSIS_PROJECT_VENDOR}.")
SET(AQSIS_PROJECT_COPYRIGHT_OTHER "The RenderMan(R) Interface Procedures and Protocol are Copyright 1988, 1989, 2000, 2005 Pixar. All Rights Reserved.")

if(WIN32)
	# There is a bug in NSI that does not handle full unix paths properly. Make
	# sure there is at least one set of four (4) backlasshes.
	#set(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
	set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\aqsis_install.exe")
	set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Aqsis")
	set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\community.aqsis.org")
	set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.aqsis.org")
	set(CPACK_NSIS_CONTACT "packages@aqsis.org")
	set(CPACK_NSIS_MODIFY_PATH ON)
	set(CPACK_SOURCE_GENERATOR "ZIP")
	# Windows NSIS Packaging
	add_subdirectory(win/nsis)

elseif(APPLE)
	set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
	set(CPACK_GENERATOR "TGZ")
	set(CPACK_SOURCE_GENERATOR "TGZ")
	# OSX bundle
	add_subdirectory(macosx/bundle)

elseif(UNIX)
	set(CPACK_SOURCE_PACKAGE_FILE_NAME
		"${AQSIS_PROJECT_NAME_BASIC}-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}")
	set(CPACK_GENERATOR "TGZ")
	set(CPACK_SOURCE_GENERATOR "TGZ")
	# Linux SPEC Packaging
	add_subdirectory(linux/rpm)

endif()


#--------------------------------------------------
# Generic Packaging setup
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${AQSIS_PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "${AQSIS_PROJECT_VENDOR}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README")
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_BUILD}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${AQSIS_PROJECT_NAME_SHORT}-${VERSION_MAJOR}.${VERSION_MINOR}")

include(CPack)
