EXTRA_DIST = librib2ri.dsp
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


lib_LTLIBRARIES =  librib2ri.la

librib2ri_la_SOURCES = librib2ri.cpp librib2ri.h
librib2ri_la_LDFLAGS = -rpath $(libdir)

pkginclude_HEADERS = librib2ri.h

INCLUDES = -I$(top_srcdir)/librib2 -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix

