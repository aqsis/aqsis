EXTRA_DIST = bmp2tif.dsp bmp2tif.vcproj
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


pkglib_LTLIBRARIES = libbmp2tif.la

libbmp2tif_la_SOURCES = bmp2tif.c
libbmp2tif_la_LIBADD = ../common/libcommon.la ../jpg2tif/libjpg2tif.la

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix
