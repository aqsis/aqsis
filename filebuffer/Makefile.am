bin_PROGRAMS = filebuffer

filebuffer_SOURCES = filebuffer.cpp
filebuffer_LDADD = $(top_builddir)/libdd/libdd.la $(top_builddir)/libaqsistypes/libaqsistypes.la -ltiff

CXXFLAGS = -g -Wall
INCLUDES = -I$(top_srcdir)/libddmsock -I$(top_srcdir)/libdd -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix 

EXTRA_DIST = filebuffer.dsp 

