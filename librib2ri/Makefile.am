EXTRA_DIST = librib2ri.dsp

lib_LTLIBRARIES =  librib2ri.la

librib2ri_la_SOURCES = librib2ri.cpp librib2ri.h
librib2ri_la_LDFLAGS = -rpath $(libdir)

include_HEADERS = librib2ri.h

if DEBUG
CXXFLAGS = -g -Wall
else
CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/librib2 -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix

