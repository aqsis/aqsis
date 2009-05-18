project(core)

# Generate extra files here.  Extra stuff in this directory is...
add_subproject(api)
add_subproject(ddmanager)
add_subproject(geometry)
#add_subproject(raytrace)
add_subproject(reyes)
add_subproject(texturing_old)

# FIXME
#include_subproject(rib2ri)

set(core_srcs
	${api_srcs}
	${ddmanager_srcs}
	${geometry_srcs}
	${reyes_srcs}
	${texturing_old_srcs}
)
set(core_hdrs
	${api_hdrs}
	${ddmanager_hdrs}
	${geometry_hdrs}
	${reyes_hdrs}
	${texturing_old_hdrs}
)

include_directories(${AQSIS_TIFF_INCLUDE_DIR})
# Optional macro definitions for non-SHARE macros.
add_definitions()

aqsis_add_library(target_? ${core_srcs} ${core_hdrs}
	COMPILE_DEFINITIONS ?_EXPORTS
	DEPENDS ?
	LINK_LIBRARIES ?
)

aqsis_install_targets(target_?)

