EXTRA_DIST = png2tif.dsp png2tif.vcproj
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


pkglib_LTLIBRARIES = libpng2tif.la

libpng2tif_la_SOURCES = png2tif.c
libpng2tif_la_LIBADD  = ../common/libcommon.la ../ppm2tif/libppm2tif.la

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix
