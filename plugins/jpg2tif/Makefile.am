EXTRA_DIST = jpg2tif.dsp jpg2tif.vcproj
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


pkglib_LTLIBRARIES = libjpg2tif.la

libjpg2tif_la_SOURCES = jpg2tif.c
libjpg2tif_la_LIBADD  = ../common/libcommon.la @JPEG_FLAGS@

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix @JPEG_INC@
