EXTRA_DIST = libraytrace.dsp libraytrace.vcproj

AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@

lib_LTLIBRARIES = libraytrace.la
INCLUDES = -I$(top_srcdir)/boost -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/render @AQSIS_BOOST_CPPFLAGS@

libraytrace_la_SOURCES = raytrace.cpp raytrace.h

pkginclude_HEADERS =

