EXTRA_DIST = ppm2tif.dsp

lib_LTLIBRARIES = libppm2tif.la

libppm2tif_la_SOURCES = ppm2tif.c

if DEBUG
CXXFLAGS = -g -Wall
else
CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I@TIFF_INC@

LDFLAGS = @LDFLAGS@ -L@TIFF_LIB@
