project(hairgen)

set(hairgen_srcs
	emitter.cpp
	hair.cpp
	parenthairs.cpp
	kdtree/kdtree2.cpp
)

add_library(hairgen MODULE ${hairgen_srcs})

set(linklibs aqsis_math aqsis_util aqsis_ribparse)
if(NOT UNIX OR APPLE)
	# We need to explicitly link to aqsis_core for the Ri* functions, except
	# on most unix platforms which find the unresolved symbols at load time.
	list(APPEND linklibs aqsis_core)
endif()
target_link_libraries(hairgen ${linklibs})

set_target_properties(hairgen PROPERTIES PREFIX "")

# TODO: Using DISPLAYSDIR here isn't nice... need to fix this, perhaps with a
# PLUGINDIR variable?
install(TARGETS hairgen DESTINATION ${DISPLAYSDIR} COMPONENT main)
