bin_PROGRAMS = filebuffer

filebuffer_SOURCES = filebuffer.cpp
filebuffer_LDADD = $(top_builddir)/libdd/libdd.la $(top_builddir)/libaqsistypes/libaqsistypes.la -ltiff -llog4cpp

if DEBUG
AM_CXXFLAGS = -g -Wall
else
AM_CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libddmsock -I$(top_srcdir)/libdd -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I@TIFF_INC@

LDFLAGS = @LDFLAGS@ -L@TIFF_LIB@

EXTRA_DIST = filebuffer.dsp 

