bin_PROGRAMS = aqsis 

aqsis_SOURCES = aqsis.cpp
aqsis_LDADD = $(top_builddir)/librib2/librib2.la $(top_builddir)/librib2ri/librib2ri.la $(top_builddir)/render/libaqsis.la $(top_builddir)/libddmsimple/libddmsimple.la $(top_builddir)/libaqsistypes/libaqsistypes.la -ltiff

CXXFLAGS = -g -Wall
INCLUDES = -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/librib2 -I$(top_srcdir)/librib2ri 

