noinst_LTLIBRARIES = libcommon.la
noinst_HEADERS = pixelsave.h

AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@ -static

libcommon_la_SOURCES = pixelsave.c
libcommon_la_LDFLAGS = @AQSIS_TIFF_LDFLAGS@

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix @AQSIS_TIFF_LDFLAGS@
