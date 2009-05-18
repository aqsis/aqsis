project(headers)

# FIXME inc_install_dir is the base of the header install tree.
#set(inc_install_dir "${INCLUDEDIR}/aqsis")
set(inc_install_dir "${CMAKE_INSTALL_PREFIX}/include/aqsis")

add_subdirectory(ri)

#------------------------------------------------------------------------------
# Create generated files and install files from this directory.

# create config.h
include(CheckIncludeFiles)
check_include_files(stdint.h AQSIS_HAVE_STDINT_H)
if(WIN32)
	set(AQSIS_SYSTEM_TYPE "WIN32")
elseif(UNIX)
	set(AQSIS_SYSTEM_TYPE "POSIX")
else()
	message(SEND_ERROR "Unknown platform!")
endif()
set(config_h_name "${PROJECT_BINARY_DIR}/config.h")
configure_file(config.h.in.cmake ${config_h_name})

# Create version.h
set(version_h_name "${PROJECT_BINARY_DIR}/version.h")
configure_file(version.h.in.cmake ${version_h_name})


# Install local headers
set(install_hdrs
	iribparser.h
	aqsis.h
	${config_h_name}
	${version_h_name}
)

install(FILES ${install_hdrs} DESTINATION ${INCLUDEDIR})

#------------------------------------------------------------------------------

# Install subdirectories of header files.
install(DIRECTORY math DESTINATION ${inc_install_dir})
install(DIRECTORY util DESTINATION ${inc_install_dir})
install(DIRECTORY riutil DESTINATION ${inc_install_dir})
#install(DIRECTORY tex DESTINATION ${inc_install_dir})

