SUBDIRS = posix

lib_LTLIBRARIES = libaqsis.la

libaqsis_la_SOURCES = attributes.cpp bitvector.cpp bound.cpp cellnoise.cpp color.cpp context.cpp file.cpp imagebuffer.cpp lights.cpp matrix.cpp messages.cpp micropolygon.cpp noise.cpp nurbs.cpp options.cpp parameters.cpp patch.cpp polygon.cpp quadrics.cpp render.cpp renderer.cpp ri.cpp shadeops.cpp shaderexecenv.cpp shaders.cpp shaderstack.cpp shadervariable.cpp shadervm.cpp spline.cpp sstring.cpp stats.cpp subdivision.cpp surface.cpp symbols.cpp texturemap.cpp transform.cpp vector2d.cpp vector3d.cpp vector4d.cpp
libaqsis_la_INCLUDES = aqsis.h aqsis_types.h attributes.h bilinear.h bison.h bitvector.h bound.h cellnoise.h color.h context.h exception.h file.h flexskel.h globals.h imagebuffer.h irenderer.h lights.h list.h matrix.h memorypool.h messages.h micropolygon.h motion.h noise.h nommgr.h nurbs.h options.h parameters.h patch.h polygon.h quadrics.h random.h render.h renderer.h ri.h scene.h shaderexecenv.h shaders.h shaderstack.h shadervariable.h shadervm.h spline.h sstring.h stats.h subdivision.h surface.h symbols.h texturemap.h transform.h vector2d.h vector3d.h vector4d.h version.h
libaqsis_la_LIBADD = posix/intel/libplatform.la

CXXFLAGS = -g -Wall
INCLUDES = -I./posix/intel

