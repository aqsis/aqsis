project(tex)

add_subproject(buffers)
add_subproject(filtering)
add_subproject(io)
add_subproject(maketexture)
add_subproject(pointcloud)

set(tex_srcs ${buffers_srcs} ${filtering_srcs}
	${io_srcs} ${maketexture_srcs} ${pointcloud_srcs}
)

set(tex_hdrs ${buffers_hdrs} ${filtering_hdrs}
	${io_hdrs} ${maketexture_hdrs} ${pointcloud_hdrs}
)
source_group("Header files" FILES ${tex_hdrs})

set(linklibs ${io_linklibs})
if(AQSIS_USE_OPENEXR)
    include_directories(${AQSIS_OPENEXR_INCLUDE_DIR})
    add_definitions(-DUSE_OPENEXR)
	list(APPEND linklibs ${AQSIS_OPENEXR_LIBRARIES})
endif()

aqsis_add_library(aqsis_tex ${tex_srcs} ${tex_hdrs}
	COMPILE_DEFINITIONS AQSIS_TEX_EXPORTS
	LINK_LIBRARIES aqsis_math aqsis_util ${linklibs}
)

aqsis_install_targets(aqsis_tex)


#--------------------------------------------------
# Testing
set(tex_test_srcs ${buffers_test_srcs} ${filtering_test_srcs}
	${io_test_srcs} ${maketexture_test_srcs} ${pointcloud_test_srcs}
)
aqsis_add_tests(${tex_test_srcs} LINK_LIBRARIES aqsis_tex)
