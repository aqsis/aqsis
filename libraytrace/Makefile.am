EXTRA_DIST = libraytrace.dsp

AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@

lib_LTLIBRARIES = libraytrace.la
INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/render

libraytrace_la_SOURCES = raytrace.cpp raytrace.h
libraytrace_la_LIBADD = $(top_builddir)/libaqsistypes/posix/libplatform.la

pkginclude_HEADERS =

