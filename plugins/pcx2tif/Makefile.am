EXTRA_DIST = pcx2tif.dsp pcx2tif.vcproj
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


pkglib_LTLIBRARIES = libpcx2tif.la

libpcx2tif_la_SOURCES = pcx2tif.c
libpcx2tif_la_LIBADD  = ../common/libcommon.la

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix
