SUBDIRS = . tests
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@

AM_YFLAGS=-d

EXTRA_DIST = librib2.dsp parser.cxx parser.h scanner.cxx

lib_LTLIBRARIES =  librib2.la

librib2_la_SOURCES = parser.yxx scanner.lxx bdec.cpp bdec.h librib.cpp librib.h libribtypes.h parserstate.h
librib2_la_LDFLAGS = -lz

pkginclude_HEADERS = librib.h parserstate.h

INCLUDES = -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix

