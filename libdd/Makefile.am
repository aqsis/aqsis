EXTRA_DIST = libdd.dsp

lib_LTLIBRARIES = libdd.la

libdd_la_SOURCES = dd.cpp dd.h 

if DEBUG
CXXFLAGS = -g -Wall
else
CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libddmsock
 

