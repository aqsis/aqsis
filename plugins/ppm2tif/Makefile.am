EXTRA_DIST = ppm2tif.dsp

lib_LTLIBRARIES = libppm2tif.la

libppm2tif_la_SOURCES = ppm2tif.c
libppm2tif_la_LIBADD  = ../common/libcommon.la
libppm2tif_la_LDFLAGS = -rpath $(libdir)

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix 
