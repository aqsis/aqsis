EXTRA_DIST = gif2tif.dsp

lib_LTLIBRARIES = libgif2tif.la

libgif2tif_la_SOURCES = gif2tif.c

if DEBUG
CXXFLAGS = -g -Wall
else
CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I@TIFF_INC@

LDFLAGS = @LDFLAGS@ -L@TIFF_LIB@
