SUBDIRS = . tests

bin_PROGRAMS = aqsl

EXTRA_DIST = aqsl.dsp 

aqsl_SOURCES = aqslcomp.cpp
aqsl_LDADD = $(top_builddir)/libslparse/libslparse.la $(top_builddir)/libcodegenvm/libcodegenvm.la $(top_builddir)/libaqsistypes/libaqsistypes.la $(top_builddir)/libargparse/libargparse.la

INCLUDES = -I$(top_srcdir)/libslparse -I$(top_srcdir)/libcodegenvm -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libargparse

