EXTRA_DIST = display.dsp

AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@

lib_LTLIBRARIES = libdisplay.la

libdisplay_la_SOURCES = \
display.cpp \
display.h \
displayhelpers.c

libdisplay_la_LIBADD = @AQSIS_FLTK_LIBS@ @TIFF_FLAGS@

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/boost -I$(top_srcdir)/libddmanager -I$(top_srcdir)/render @AQSIS_FLTK_CFLAGS@ @TIFF_INC@

