EXTRA_DIST = librib2stream.dsp

lib_LTLIBRARIES =  librib2stream.la

librib2stream_la_SOURCES = librib2stream.cpp librib2stream.h
librib2stream_la_LDFLAGS = -rpath $(libdir)

include_HEADERS = librib2stream.h

if DEBUG
CXXFLAGS = -g -Wall
else
CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/librib2

