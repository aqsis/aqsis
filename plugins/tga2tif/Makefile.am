EXTRA_DIST = tga2tif.dsp

lib_LTLIBRARIES = libtga2tif.la

libtga2tif_la_SOURCES = tga2tif.c
libtga2tif_la_LIBADD = ../common/libcommon.la
libtga2tif_la_LDFLAGS = -rpath $(libdir)

if DEBUG
AM_CXXFLAGS = -g -Wall
else
AM_CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I@TIFF_INC@

LDFLAGS = @LDFLAGS@ -L@TIFF_LIB@
