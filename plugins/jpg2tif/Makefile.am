EXTRA_DIST = jpg2tif.dsp

lib_LTLIBRARIES = libjpg2tif.la

libjpg2tif_la_SOURCES = jpg2tif.c
libjpg2tif_la_LIBADD  = ../common/libcommon.la
libjpg2tif_la_LDFLAGS = -rpath $(libdir)

if DEBUG
CXXFLAGS = -g -Wall
else
CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I@TIFF_INC@

LDFLAGS = @LDFLAGS@ -L@TIFF_LIB@
