EXTRA_DIST = libslxargs.dsp
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


lib_LTLIBRARIES = libslxargs.la

libslxargs_la_SOURCES = slx.cpp slx.h rcdummy.cpp
libslxargs_la_LDFLAGS = -rpath $(libdir)

include_HEADERS = slx.h

INCLUDES = -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix  -I$(top_srcdir)/librib2ri -I$(top_srcdir)/librib2 -I$(top_srcdir)/libshadervm -I$(top_srcdir)/libshaderexecenv
 

