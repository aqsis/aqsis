lib_LTLIBRARIES = libddmsock.la

libddmsock_la_SOURCES = ddmsock.cpp ddmsock.h displaydriver.h
libddmsock_la_LDFLAGS = -rpath $(libdir)

INCLUDES = -I$(top_srcdir)/render -I$(top_srcdir)/libddmsock -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libshaderexecenv

EXTRA_DIST = libddmsock.dsp 
