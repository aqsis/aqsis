SUBDIRS = . tests
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


bin_PROGRAMS = aqsl

EXTRA_DIST = aqsl.dsp aqsl.vcproj

aqsl_SOURCES = aqslcomp.cpp
aqsl_LDADD = $(top_builddir)/libslparse/libslparse.la $(top_builddir)/libcodegenvm/libcodegenvm.la $(top_builddir)/libaqsistypes/libaqsistypes.la $(top_builddir)/libargparse/libargparse.la $(top_builddir)/slpp/libslpp.la @AQSIS_PLUGIN_LDFLAGS@

INCLUDES = -I$(top_srcdir)/libslparse -I$(top_srcdir)/libcodegenvm -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libargparse

