project(tex)

add_subproject(buffers)
add_subproject(filtering)
add_subproject(io)
add_subproject(maketexture)
add_subproject(pointcloud)

set(tex_srcs ${buffers_srcs} ${filtering_srcs}
	${io_srcs} ${maketexture_srcs} ${pointcloud_srcs}
)
set(tex_test_srcs ${buffers_test_srcs} ${filtering_test_srcs}
	${io_test_srcs} ${maketexture_test_srcs} ${pointcloud_test_srcs}
)

set(tex_hdrs ${buffers_hdrs} ${filtering_hdrs}
	${io_hdrs} ${maketexture_hdrs} ${pointcloud_hdrs}
)
source_group("Header files" FILES ${tex_hdrs})

set(linklibs ${io_linklibs})
if(AQSIS_USE_OPENEXR)
    include_directories(${AQSIS_OPENEXR_INCLUDE_DIR} "${AQSIS_OPENEXR_INCLUDE_DIR}/OpenEXR")
    add_definitions(-DUSE_OPENEXR)
	list(APPEND linklibs ${AQSIS_OPENEXR_LIBRARIES})
endif()
if(AQSIS_USE_PNG)
  include_directories(${AQSIS_PNG_INCLUDE_DIR} ${AQSIS_ZLIB_INCLUDE_DIR})
	add_definitions(-DAQSIS_USE_PNG)
endif()
list(APPEND linklibs ${AQSIS_ZLIB_LIBRARIES})

aqsis_add_library(aqsis_tex ${tex_srcs} ${tex_hdrs}
	TEST_SOURCES ${tex_test_srcs}
	COMPILE_DEFINITIONS AQSIS_TEX_EXPORTS
	LINK_LIBRARIES aqsis_math aqsis_util ${linklibs}
)

aqsis_install_targets(aqsis_tex)

