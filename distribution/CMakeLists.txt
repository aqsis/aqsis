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
	set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
		!addincludedir \\\"${CMAKE_SOURCE_DIR}/distribution/win/nsis\\\"
		!include custom_install.nsi")
	set(CPACK_ALL_INSTALL_TYPES Full Minimal)
	set(CPACK_COMPONENT_MAIN_INSTALL_TYPES Full Minimal)
	set(CPACK_COMPONENT_DEVELOPMENT_INSTALL_TYPES Full)
	set(CPACK_COMPONENT_EXAMPLES_INSTALL_TYPES Full)
	set(CPACK_COMPONENT_SHADERS_INSTALL_TYPES Full)
	set(CPACK_COMPONENT_PLUGINS_INSTALL_TYPES Full)
	set(CPACK_GENERATOR "NSIS")
	set(CPACK_SOURCE_GENERATOR "ZIP")
	# Cusotm Windows NSIS installer
	#add_subdirectory(win/nsis)

elseif(APPLE)
	set(BUNDLEDIR "${CMAKE_BINARY_DIR}/bundle"
		CACHE STRING "Staging location for bundles (relative to CMAKE_BINARY_DIR)"
		)
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
	#add_subdirectory(macosx/bundle)

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
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README")
set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_SOURCE_DIR}/README")
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_BUILD}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${AQSIS_PROJECT_NAME_SHORT}")

set(CPACK_COMPONENTS_ALL main docs examples shaders plugins development Unspecified)
set(CPACK_COMPONENT_MAIN_REQUIRED TRUE)
set(CPACK_COMPONENT_UNSPECIFIED_REQUIRED TRUE)
set(CPACK_COMPONENT_UNSPECIFIED_HIDDEN TRUE)
set(CPACK_COMPONENT_EXAMPLES_GROUP "content")
set(CPACK_COMPONENT_SHADERS_GROUP "content")
set(CPACK_COMPONENT_MAIN_DESCRIPTION
	"Rendering application and essential files only")
set(CPACK_COMPONENT_GROUP_CONTENT_DESCRIPTION
	"Examples and shader source files")
set(CPACK_COMPONENT_DOCS_DESCRIPTION
	"User manual and guides (.pdf)")
set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION
	"Example files (.rib)")
set(CPACK_COMPONENT_SHADERS_DESCRIPTION
	"Generic shader source files (.sl)")
set(CPACK_COMPONENT_PLUGINS_DESCRIPTION
	"Plugins for external applications")
set(CPACK_COMPONENT_DEVELOPMENT_DESCRIPTION
	"Include and library files for software development")

set(CPACK_SOURCE_IGNORE_FILES
	".*/\\\\.git/"
	)

include(CPack)

# This is a bit of a hack.
# The fixup_bundle macro will 'fixup' all exes and dlls in the specified folder, 
# but you must specify an executable as a start point. I tried doing it for each
# executable, but it then 'fixes' up every executable each time, which is time 
# consuming and results in the same thing. 
# By moving it here, we guarantee it happens after the files are installed, as this
# is add_subdirectory'd last, and I arbitrarily choose piqsl, as if QT is enabled,
# all QT apps are build anyway at the moment.
get_directory_property(display_DISPLAYLIB DIRECTORY ../tools/displays DEFINITION file_display_name)
get_directory_property(d_exr_DISPLAYLIB DIRECTORY ../tools/displays DEFINITION exr_display_name)
get_directory_property(d_bmp_DISPLAYLIB DIRECTORY ../tools/displays DEFINITION bmp_display_name)
get_directory_property(d_xpm_DISPLAYLIB DIRECTORY ../tools/displays DEFINITION xpm_display_name)
get_directory_property(piqsl_DISPLAYLIB DIRECTORY ../tools/displays DEFINITION piqsl_display_name)

if(APPLE OR WIN32)
	set(DIRS "${QT_BINARY_DIR}" "${Boost_LIBRARY_DIRS}")
	set(APPS "\${CMAKE_INSTALL_PREFIX}/${BINDIR}/piqsl${CMAKE_EXECUTABLE_SUFFIX}")
	set(PLUGINS "\${CMAKE_INSTALL_PREFIX}/${PLUGINDIR}/${display_DISPLAYLIB}"
							"\${CMAKE_INSTALL_PREFIX}/${PLUGINDIR}/${d_exr_DISPLAYLIB}"
							"\${CMAKE_INSTALL_PREFIX}/${PLUGINDIR}/${d_bmp_DISPLAYLIB}"
							"\${CMAKE_INSTALL_PREFIX}/${PLUGINDIR}/${d_xpm_DISPLAYLIB}"
							"\${CMAKE_INSTALL_PREFIX}/${PLUGINDIR}/${piqsl_DISPLAYLIB}"
							)

	INSTALL(CODE "
		include(BundleUtilities)
		fixup_bundle(\"${APPS}\"   \"${PLUGINS}\"   \"${DIRS}\")
		" COMPONENT main)
endif()
