include_subproject(partio)

set(pointrender_srcs
    microbuf_proj_func.cpp
    MicroBuf.cpp
    OcclusionIntegrator.cpp
    RadiosityIntegrator.cpp
    diffuse/DiffusePointOctree.cpp
    diffuse/DiffusePointOctreeCache.cpp
)

make_absolute(pointrender_srcs ${pointrender_SOURCE_DIR})
list(APPEND pointrender_srcs ${partio_srcs})
list(APPEND pointrender_srcs ${pngpp_srcs})

set(pointrender_hdrs
    microbuf_proj_func.h
    MicroBuf.h
    OcclusionIntegrator.h
    RadiosityIntegrator.h
    diffuse/DiffusePointOctree.h
    diffuse/DiffusePointOctreeCache.h
)

make_absolute(pointrender_hdrs ${pointrender_SOURCE_DIR})
source_group("Header Files" FILES ${pointrender_hdrs})

include_directories(${pointrender_SOURCE_DIR})

set(pointrender_libs ${partio_libs} ${math_libs})
