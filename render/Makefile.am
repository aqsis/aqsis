SUBDIRS = macosx .
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@

EXTRA_DIST = libaqsis.dsp libaqsis.def

lib_LTLIBRARIES = libaqsis.la

libaqsis_la_SOURCES = shadowmap.cpp iattributes.h attributes.cpp attributes.h bilinear.h bound.cpp bound.h environment.cpp graphicsstate.cpp graphicsstate.h iddmanager.h imagebuffer.cpp imagebuffer.h ilightsource.h lights.cpp lights.h messages.cpp messages.h micropolygon.cpp micropolygon.h motion.h nurbs.cpp nurbs.h options.cpp options.h parameters.cpp parameters.h patch.cpp patch.h polygon.cpp polygon.h quadrics.cpp quadrics.h points.cpp points.h curves.cpp curves.h render.cpp render.h irenderer.h renderer.cpp renderer.h ri.cpp ri.h rifile.h ishader.h shaders.cpp shaders.h stats.cpp stats.h isurface.h surface.cpp surface.h symbols.cpp symbols.h teapot.cpp teapot.h itexturemap.h texturemap.cpp texturemap.h itransform.h transform.cpp transform.h version.h trimcurve.cpp trimcurve.h csgtree.h csgtree.cpp genpoly.h genpoly.cpp imagers.h imagers.cpp ishaderexecenv.h ishaderdata.h forwarddiff.h occlusion.h occlusion.cpp subdivision2.cpp lath.cpp subdivision2.h lath.h converter.cpp converter.h kdtree.h idsoshadeops.h bucket.cpp bucket.h imagepixel.cpp imagepixel.h procedural.cpp procedural.h focus.h inlineparse.cpp inlineparse.h

libaqsis_la_LIBADD = $(top_builddir)/librib2ri/librib2ri.la $(top_builddir)/librib2/librib2.la $(top_builddir)/libddmsock/libddmsock.la $(top_builddir)/libshadervm/libshadervm.la $(top_builddir)/libshaderexecenv/libshaderexecenv.la $(top_builddir)/libaqsistypes/libaqsistypes.la @TIFF_FLAGS@

pkginclude_HEADERS = ri.h shadeop.h

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libshadervm -I$(top_srcdir)/libshaderexecenv -I$(top_srcdir)/librib2 -I$(top_srcdir)/librib2ri @TIFF_INC@


