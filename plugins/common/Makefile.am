noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = pixelsave.c pixelsave.h
libcommon_la_LDFLAGS = -rpath $(libdir) -static

if DEBUG
AM_CXXFLAGS = -g -Wall
else
AM_CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I@TIFF_INC@

