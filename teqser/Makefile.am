bin_PROGRAMS = teqser 

teqser_SOURCES = teqser.cpp
teqser_LDADD = $(top_builddir)/render/libaqsis.la $(top_builddir)/libddmsimple/libddmsimple.la $(top_builddir)/libaqsistypes/libaqsistypes.la -ltiff -largparse

if DEBUG
CXXFLAGS = -g -Wall
else
CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/render

EXTRA_DIST = teqser.dsp

