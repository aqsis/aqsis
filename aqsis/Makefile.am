bin_PROGRAMS = aqsis 

aqsis_SOURCES = aqsis.cpp

CXXFLAGS = -g -Wall
INCLUDES = -I$(top_builddir)/render -I$(top_builddir)/render/posix/intel -I$(top_builddir)/file
LDFLAGS = 
LDADD = $(top_builddir)/render/libaqsis.la $(top_builddir)/file/libfile.la -ltiff 



