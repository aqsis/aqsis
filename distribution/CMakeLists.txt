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
	set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\eqsl.exe")
	set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} ${AQSIS_PROJECT_NAME} ${AQSIS_VERSION} ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}")
	set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\community.aqsis.org")
	set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.aqsis.org")
	set(CPACK_NSIS_CONTACT "packages@aqsis.org")
	set(CPACK_NSIS_MODIFY_PATH ON)
	set(CPACK_ALL_INSTALL_TYPES Full Minimal)
	set(CPACK_COMPONENT_${BINDOR}_INSTALL_TYPES Full Minimal)
	set(CPACK_COMPONENT_${INCLUDEDIR}_INSTALL_TYPES Full)
	set(CPACK_COMPONENT_${LIBDIR}_INSTALL_TYPES Full)
	set(CPACK_GENERATOR "NSIS")
	set(CPACK_SOURCE_GENERATOR "ZIP")
	# Cusotm Windows NSIS installer
	#add_subdirectory(win/nsis)

elseif(APPLE)
	set(CPACK_PACKAGE_FILE_NAME
		"${AQSIS_PROJECT_NAME_SHORT}-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}-${CMAKE_SYSTEM_NAME}")
	#set(CPACK_PACKAGE_ICON "")
	set(CPACK_BUNDLE_NAME "${AQSIS_PROJECT_NAME_SHORT}")
	set(CPACK_BUNDLE_ICON "${CMAKE_SOURCE_DIR}/distribution/macosx/application.icns")
	set(CPACK_BUNDLE_PLIST "${BUNDLEDIR}/Info.plist")
	set(CPACK_BUNDLE_STARTUP_COMMAND "applet")
	set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}") # DEPRECATED ENTRY?
	set(CPACK_GENERATOR "TGZ")
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

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${AQSIS_PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "${AQSIS_PROJECT_VENDOR}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_BUILD}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${AQSIS_PROJECT_NAME_SHORT}")

set(CPACK_COMPONENTS_ALL main examples shaders development)
set(CPACK_COMPONENT_MAIN_REQUIRED)
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
