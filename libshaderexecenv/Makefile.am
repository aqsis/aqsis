EXTRA_DIST = libshaderexecenv.dsp 

lib_LTLIBRARIES = libshaderexecenv.la

libshaderexecenv_la_SOURCES = shaderexecenv.cpp shaderexecenv.h shadeops.cpp
libshaderexecenv_la_LDFLAGS = -rpath $(libdir)

if DEBUG
AM_CXXFLAGS = -g -Wall
else
AM_CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/render -I$(top_srcdir)/libshadervm


