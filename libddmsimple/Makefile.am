EXTRA_DIST = libddmsimple.dsp

lib_LTLIBRARIES = libddmsimple.la

libddmsimple_la_SOURCES = ddmsimple.cpp ddmsimple.h 
libddmsimple_la_LDFLAGS = -rpath $(libdir)

if DEBUG
AM_CXXFLAGS = -g -Wall
else
AM_CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libshaderexecenv -I@TIFF_INC@

LDFLAGS = @LDFLAGS@ -L@TIFF_LIB@
