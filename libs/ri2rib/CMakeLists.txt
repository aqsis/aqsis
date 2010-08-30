project(ri2rib)

set(ri2rib_srcs
	../riutil/ri2ricxx.cpp
	../riutil/risyms.cpp
	ri2rib.cpp
)

aqsis_add_library(aqsis_ri2rib ${ri2rib_srcs}
	COMPILE_DEFINITIONS AQSIS_RI_EXPORTS
	LINK_LIBRARIES aqsis_riutil
)

aqsis_install_targets(aqsis_ri2rib)

if(aqsis_enable_testing)
	add_executable(ri2rib_test ri2rib_test.cpp)
	target_link_libraries(ri2rib_test aqsis_ri2rib)
endif()
