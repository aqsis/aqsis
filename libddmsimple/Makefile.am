EXTRA_DIST = libddmsimple.dsp
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


lib_LTLIBRARIES = libddmsimple.la

libddmsimple_la_SOURCES = ddmsimple.cpp ddmsimple.h 
libddmsimple_la_LDFLAGS = -rpath $(libdir) @TIFF_FLAGS@

INCLUDES = -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libshaderexecenv @TIFF_INC@

