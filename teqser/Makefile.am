EXTRA_DIST = teqser.dsp teqser.vcproj

AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@

bin_PROGRAMS = teqser 
teqser_SOURCES = teqser.cpp
teqser_LDADD = $(top_builddir)/render/libaqsis.la $(top_builddir)/libraytrace/libraytrace.la $(top_builddir)/libddmanager/libddmanager.la $(top_builddir)/libshadervm/libshadervm.la $(top_builddir)/libshaderexecenv/libshaderexecenv.la $(top_builddir)/librib2/librib2.la $(top_builddir)/librib2ri/librib2ri.la $(top_builddir)/libaqsistypes/libaqsistypes.la $(top_builddir)/libargparse/libargparse.la @AQSIS_PLUGIN_LDFLAGS@

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/render -I$(top_srcdir)/libargparse

