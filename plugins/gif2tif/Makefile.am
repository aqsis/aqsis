EXTRA_DIST = gif2tif.dsp

lib_LTLIBRARIES = libgif2tif.la

libgif2tif_la_SOURCES = gif2tif.c 
libgif2tif_la_LIBADD = ../common/libcommon.la 
libgif2tif_la_LDFLAGS = -rpath $(libdir)

if DEBUG
AM_CXXFLAGS = -g -Wall
else
AM_CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I@TIFF_INC@

LDFLAGS = @LDFLAGS@ -L@TIFF_LIB@ 
