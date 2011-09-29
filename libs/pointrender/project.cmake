include_subproject(partio)

set(pointrender_srcs
    microbuffer.cpp
    pointcontainer.cpp
)
make_absolute(pointrender_srcs ${pointrender_SOURCE_DIR})
list(APPEND pointrender_srcs ${partio_srcs})

set(pointrender_hdrs
    microbuffer.h
    pointcontainer.h
)
make_absolute(pointrender_hdrs ${pointrender_SOURCE_DIR})
source_group("Header Files" FILES ${pointrender_hdrs})

include_directories(${pointrender_SOURCE_DIR})

set(pointrender_libs ${partio_libs})
