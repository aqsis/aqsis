EXTRA_DIST = tga2tif.dsp

lib_LTLIBRARIES = libtga2tif.la

libtga2tif_la_SOURCES = tga2tif.c
libtga2tif_la_LIBADD = ../common/libcommon.la
libtga2tif_la_LDFLAGS = -rpath $(libdir)

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix
