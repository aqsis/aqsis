project(hairgen)

set(hairgen_srcs
	emitter.cpp
	hair.cpp
	parenthairs.cpp
	kdtree/kdtree2.cpp
)

set(linklibs aqsis_math aqsis_util aqsis_riutil)
if(NOT UNIX OR APPLE)
	# We need to explicitly link to aqsis_core for the Ri* functions, except
	# on most unix platforms which find the unresolved symbols at load time.
	list(APPEND linklibs aqsis_core)
endif()

aqsis_add_library(hairgen ${hairgen_srcs} PLUGIN
	LINK_LIBRARIES ${linklibs})

install(TARGETS hairgen DESTINATION ${PLUGINDIR} COMPONENT main)
