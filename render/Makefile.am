SUBDIRS = macosx .
EXTRA_DIST = libaqsis.dsp

lib_LTLIBRARIES = libaqsis.la

libaqsis_la_SOURCES = iattributes.h attributes.cpp attributes.h bilinear.h bound.cpp bound.h context.cpp context.h iddmanager.h imagebuffer.cpp imagebuffer.h ilightsource.h lights.cpp lights.h messages.cpp messages.h micropolygon.cpp micropolygon.h motion.h nurbs.cpp nurbs.h options.cpp options.h parameters.cpp parameters.h patch.cpp patch.h polygon.cpp polygon.h quadrics.cpp quadrics.h points.cpp points.h render.cpp render.h irenderer.h renderer.cpp renderer.h ri.cpp ri.h rifile.h ishader.h shaders.cpp shaders.h stats.cpp stats.h subdivision.cpp subdivision.h isurface.h surface.cpp surface.h symbols.cpp symbols.h teapot.cpp teapot.h itexturemap.h texturemap.cpp texturemap.h itransform.h transform.cpp transform.h version.h trimcurve.cpp trimcurve.h csgtree.h csgtree.cpp genpoly.h genpoly.cpp imagers.h imagers.cpp plugins.cpp plugins.h ishaderexecenv.h ishaderdata.h 
libaqsis_la_LDFLAGS = -rpath $(libdir)

include_HEADERS = ri.h

if DEBUG
AM_CXXFLAGS = -g -Wall
else
AM_CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libshadervm -I$(top_srcdir)/libshaderexecenv


