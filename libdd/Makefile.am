EXTRA_DIST = libdd.dsp

lib_LTLIBRARIES = libdd.la

libdd_la_SOURCES = dd.cpp dd.h 
libdd_la_LDFLAGS = -rpath $(libdir)

if DEBUG
AM_CXXFLAGS = -g -Wall
else
AM_CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libddmsock
 

