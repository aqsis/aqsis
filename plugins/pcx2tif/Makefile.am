EXTRA_DIST = pcx2tif.dsp

lib_LTLIBRARIES = libpcx2tif.la

libpcx2tif_la_SOURCES = pcx2tif.c

if DEBUG
CXXFLAGS = -g -Wall
else
CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I@TIFF_INC@

LDFLAGS = @LDFLAGS@ -L@TIFF_LIB@
