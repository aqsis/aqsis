EXTRA_DIST = pcx2tif.dsp
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


lib_LTLIBRARIES = libpcx2tif.la

libpcx2tif_la_SOURCES = pcx2tif.c
libpcx2tif_la_LIBADD  = ../common/libcommon.la
libpcx2tif_la_LDFLAGS = -rpath $(libdir)

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix
