EXTRA_DIST = libaqfile.dsp libaqfile.vcproj

AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@

lib_LTLIBRARIES = libaqfile.la
INCLUDES = -I$(top_srcdir)/boost -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix @AQSIS_BOOST_CPPFLAGS@

libaqfile_la_SOURCES = file.cpp file.h

pkginclude_HEADERS =

