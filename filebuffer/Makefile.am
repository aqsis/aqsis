bin_PROGRAMS = filebuffer
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


filebuffer_SOURCES = filebuffer.cpp
filebuffer_LDADD = $(top_builddir)/libdd/libdd.la $(top_builddir)/libaqsistypes/libaqsistypes.la @TIFF_FLAGS@ 

INCLUDES = -I$(top_srcdir)/libddmsock -I$(top_srcdir)/libdd -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix @TIFF_INC@

EXTRA_DIST = filebuffer.dsp 

