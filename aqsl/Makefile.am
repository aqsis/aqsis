SUBDIRS = . tests

bin_PROGRAMS = aqslcomp
bin_SCRIPTS = aqsl

EXTRA_DIST = aqsl aqsl.bat aqsl.dsp aqslcomp.dsp macaqsl

aqslcomp_SOURCES = aqslcomp.cpp
aqslcomp_LDADD = $(top_builddir)/libslparse/libslparse.la $(top_builddir)/libcodegenvm/libcodegenvm.la $(top_builddir)/libaqsistypes/libaqsistypes.la $(top_builddir)/libargparse/libargparse.la

INCLUDES = -I$(top_srcdir)/libslparse -I$(top_srcdir)/libcodegenvm -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libargparse

