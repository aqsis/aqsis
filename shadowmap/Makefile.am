bin_PROGRAMS = shadowmap

shadowmap_SOURCES = shadowmap.cpp
shadowmap_LDADD = $(top_builddir)/libdd/libdd.la $(top_builddir)/libaqsistypes/libaqsistypes.la -ltiff

if DEBUG
CXXFLAGS = -g -Wall
else
CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libddmsock -I$(top_srcdir)/libdd -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix 

EXTRA_DIST = shadowmap.dsp 

