noinst_LTLIBRARIES = libcommon.la
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


libcommon_la_SOURCES = pixelsave.c pixelsave.h
libcommon_la_LDFLAGS = -static @TIFF_FLAGS@

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix @TIFF_INC@

