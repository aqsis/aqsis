EXTRA_DIST = libddmanager.dsp

AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@

lib_LTLIBRARIES = libddmanager.la
libddmanager_la_SOURCES = \
ddmanager.cpp \
ddmanager.h \
ndspy.h

INCLUDES = -I$(top_srcdir)/boost -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libshaderexecenv -I$(top_srcdir)/render

