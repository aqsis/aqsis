EXTRA_DIST = librib2stream.dsp
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


lib_LTLIBRARIES =  librib2stream.la

librib2stream_la_SOURCES = librib2stream.cpp librib2stream.h
librib2stream_la_LDFLAGS = -rpath $(libdir)

include_HEADERS = librib2stream.h

INCLUDES = -I$(top_srcdir)/librib2 -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libri2rib

