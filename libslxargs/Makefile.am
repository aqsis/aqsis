EXTRA_DIST = libslxargs.dsp

lib_LTLIBRARIES = libslxargs.la

libslxargs_la_SOURCES = slx.cpp slx.h 
libslxargs_la_LDFLAGS = -rpath $(libdir)

if DEBUG
AM_CXXFLAGS = -g -Wall
else
AM_CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix  -I$(top_srcdir)/librib2ri -I$(top_srcdir)/librib2 -I$(top_srcdir)/libshadervm -I$(top_srcdir)/libshaderexecenv
 

