set(geometry_srcs
	blobby.cpp
	bunny.cpp
	cubiccurves.cpp
	curves.cpp
	jules_bloomenthal.cpp
	lath.cpp
	linearcurves.cpp
	marchingcubes.cpp
	nurbs.cpp
	patch.cpp
	points.cpp
	polygon.cpp
	procedural.cpp
	quadrics.cpp
	subdivision2.cpp
	surface.cpp
	teapot.cpp
	trimcurve.cpp
)
make_absolute(geometry_srcs ${geometry_SOURCE_DIR})

set(geometry_hdrs
	blobby.h
	bunny.h
	curves.h
	jules_bloomenthal.h
	kdtree.h
	lath.h
	lookuptable.h
	marchingcubes.h
	nurbs.h
	patch.h
	points.h
	polygon.h
	procedural.h
	quadrics.h
	subdivision2.h
	surface.h
	teapot.h
	trimcurve.h
)
make_absolute(geometry_hdrs ${geometry_SOURCE_DIR})

include_directories(${geometry_SOURCE_DIR})

