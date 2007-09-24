# There is a bug in NSI that does not handle full unix paths properly. Make
# sure there is at least one set of four (4) backlasshes.
#SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
#
# Packaging setup
#
SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\aqsis_install.exe")
SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Aqsis")
SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.aqsis.org")
SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.aqsis.org")
SET(CPACK_NSIS_CONTACT "packages@aqsis.org")
SET(CPACK_NSIS_MODIFY_PATH ON)
SET(CPACK_GENERATOR "ZIP;NSIS")
SET(CPACK_SOURCE_GENERATOR "ZIP")
#ELSE(WIN32 AND NOT UNIX)
SET(CPACK_STRIP_FILES ON)
