bin_PROGRAMS = aqsis 

aqsis_SOURCES = aqsis.cpp

CXXFLAGS = -g -Wall
INCLUDES = -I$(top_srcdir)/render -I$(top_srcdir)/render/posix/intel -I$(top_srcdir)/file
LDFLAGS = 
LDADD = $(top_builddir)/render/libaqsis.la $(top_builddir)/file/libfile.la -ltiff 



