EXTRA_DIST = pcx2tif.dsp

lib_LTLIBRARIES = libpcx2tif.la

libpcx2tif_la_SOURCES = pcx2tif.c
libpcx2tif_la_LIBADD  = ../common/libcommon.la
libpcx2tif_la_LDFLAGS = -rpath $(libdir)

if DEBUG
AM_CXXFLAGS = -g -Wall
else
AM_CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I@TIFF_INC@

LDFLAGS = @LDFLAGS@ -L@TIFF_LIB@
