EXTRA_DIST = librib2stream.dsp librib2stream.vcproj
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


lib_LTLIBRARIES =  librib2stream.la

librib2stream_la_SOURCES = librib2stream.cpp librib2stream.h

pkginclude_HEADERS = librib2stream.h

INCLUDES = -I$(top_srcdir)/librib2 -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libri2rib -I$(top_srcdir)/render

