noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = pixelsave.c pixelsave.h
libcommon_la_LDFLAGS = -rpath $(libdir) -static @TIFF_FLAGS@

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix @TIFF_INC@

