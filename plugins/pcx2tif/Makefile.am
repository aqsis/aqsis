EXTRA_DIST = pcx2tif.dsp

lib_LTLIBRARIES = libpcx2tif.la

libpcx2tif_la_SOURCES = pcx2tif.c
libpcx2tif_la_LIBADD  = ../common/libcommon.la
libpcx2tif_la_LDFLAGS = -rpath $(libdir)

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix
