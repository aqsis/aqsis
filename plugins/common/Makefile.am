noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = pixelsave.c pixelsave.h

if DEBUG
CXXFLAGS = -g -Wall
else
CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I@TIFF_INC@

