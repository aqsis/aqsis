EXTRA_DIST = gif2tif.dsp gif2tif.vcproj
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


pkglib_LTLIBRARIES = libgif2tif.la

libgif2tif_la_SOURCES = gif2tif.c 
libgif2tif_la_LIBADD = ../common/libcommon.la 

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix
