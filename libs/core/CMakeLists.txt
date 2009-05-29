project(core)

# Generate extra files here.  Extra stuff in this directory is...
add_subproject(api)
add_subproject(ddmanager)
add_subproject(geometry)
add_subproject(raytrace)
add_subproject(reyes)
add_subproject(texturing_old)

include_subproject(rib2ri)

set(core_srcs
	${api_srcs}
	${ddmanager_srcs}
	${geometry_srcs}
	${raytrace_srcs}
	${reyes_srcs}
	${texturing_old_srcs}
	${rib2ri_srcs}
)
set(core_hdrs
	${api_hdrs}
	${ddmanager_hdrs}
	${geometry_hdrs}
	${raytrace_hdrs}
	${reyes_hdrs}
	${texturing_old_hdrs}
	${rib2ri_hdrs}
)

source_group("Header Files" FILES ${core_hdrs})

include_directories(${AQSIS_TIFF_INCLUDE_DIR})

# Create list of preprocessor definitions
set(defs AQSIS_RI_EXPORTS AQSIS_CORE_EXPORTS
	"AQSIS_MAIN_CONFIG_NAME=${AQSIS_MAIN_CONFIG_NAME}")
if(AQSIS_ENABLE_MPDUMP)
	list(APPEND defs ENABLE_MPDUMP)
endif()
if(AQSIS_USE_TIMERS)
	list(APPEND defs USE_TIMERS)
endif()
if(AQSIS_ENABLE_THREADING)
	list(APPEND defs ENABLE_THREADING)
endif()
if(DEFAULT_RC_PATH)
	list(APPEND defs "DEFAULT_RC_PATH=${DEFAULT_RC_PATH}")
endif()


# Add library & install
aqsis_add_library(aqsis_core ${core_srcs} ${core_hdrs}
	COMPILE_DEFINITIONS ${defs}
	LINK_LIBRARIES aqsis_math aqsis_ribparse aqsis_shadervm
		aqsis_tex aqsis_util aqsis_riutil
		${AQSIS_TIFF_LIBRARIES} ${Boost_THREAD_LIBRARY} ${CARBON_LIBRARY}
)

aqsis_install_targets(aqsis_core COMPONENT development)


#--------------------------------------------------
# Testing
set(core_test_srcs
	${api_test_srcs}
	${reyes_test_srcs}
)

aqsis_add_tests(${core_test_srcs} LINK_LIBRARIES aqsis_core)
