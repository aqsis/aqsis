bin_PROGRAMS = teqser 

teqser_SOURCES = teqser.cpp
if PLUGINS
teqser_LDADD = $(top_builddir)/render/libaqsis.la $(top_builddir)/libddmsimple/libddmsimple.la $(top_builddir)/libshadervm/libshadervm.la $(top_builddir)/libshaderexecenv/libshaderexecenv.la $(top_builddir)/librib2/librib2.la $(top_builddir)/librib2ri/librib2ri.la $(top_builddir)/libaqsistypes/libaqsistypes.la -ltiff -largparse -ldl
else
teqser_LDADD = $(top_builddir)/render/libaqsis.la $(top_builddir)/libddmsimple/libddmsimple.la $(top_builddir)/libshadervm/libshadervm.la $(top_builddir)/libshaderexecenv/libshaderexecenv.la $(top_builddir)/librib2/librib2.la $(top_builddir)/librib2ri/librib2ri.la $(top_builddir)/libaqsistypes/libaqsistypes.la -ltiff -largparse
endif

if DEBUG
AM_CXXFLAGS = -g -Wall
else
AM_CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/render

EXTRA_DIST = teqser.dsp

