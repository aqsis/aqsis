EXTRA_DIST = bake2tif.dsp

lib_LTLIBRARIES = libbake2tif.la

libbake2tif_la_SOURCES = bake2tif.c
libbake2tif_la_LIBADD  = ../common/libcommon.la
libbake2tif_la_LDFLAGS = -rpath $(libdir)

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix 
