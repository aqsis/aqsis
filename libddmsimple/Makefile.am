EXTRA_DIST = libddmsimple.dsp

lib_LTLIBRARIES = libddmsimple.la

libddmsimple_la_SOURCES = ddmsimple.cpp ddmsimple.h 

if DEBUG
CXXFLAGS = -g -Wall
else
CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I@TIFF_INC@

LDFLAGS = @LDFLAGS@ -L@TIFF_LIB@
