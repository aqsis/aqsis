EXTRA_DIST = tga2tif.dsp tga2tif.vcproj
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


pkglib_LTLIBRARIES = libtga2tif.la

libtga2tif_la_SOURCES = tga2tif.c
libtga2tif_la_LIBADD = ../common/libcommon.la ../jpg2tif/libjpg2tif.la

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix
