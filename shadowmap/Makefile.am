bin_PROGRAMS = shadowmap
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


shadowmap_SOURCES = shadowmap.cpp
shadowmap_LDADD = $(top_builddir)/libdd/libdd.la $(top_builddir)/libaqsistypes/libaqsistypes.la @TIFF_FLAGS@

INCLUDES = -I$(top_srcdir)/boost -I$(top_srcdir)/libddmsock -I$(top_srcdir)/libdd -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/render @TIFF_INC@

EXTRA_DIST = shadowmap.dsp 

