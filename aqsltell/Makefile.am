bin_PROGRAMS = aqsltell 

aqsltell_SOURCES = aqsltell.cpp
if PLUGINS
aqsltell_LDADD = $(top_builddir)/libslxargs/libslxargs.la $(top_builddir)/libshadervm/libshadervm.la $(top_builddir)/libshaderexecenv/libshaderexecenv.la $(top_builddir)/libaqsistypes/libaqsistypes.la -largparse -ldl
else
aqsltell_LDADD = $(top_builddir)/libslxargs/libslxargs.la $(top_builddir)/libshadervm/libshadervm.la $(top_builddir)/libshaderexecenv/libshaderexecenv.la $(top_builddir)/libaqsistypes/libaqsistypes.la -largparse
endif

if DEBUG
AM_CXXFLAGS = -g -Wall
else
AM_CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libshadervm -I$(top_srcdir)/libshaderexecenv -I$(top_srcdir)/libslxargs

EXTRA_DIST = aqsltell.dsp

