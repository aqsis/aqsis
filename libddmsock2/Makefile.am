lib_LTLIBRARIES = libddmsock2.la

libddmsock2_la_SOURCES = bucketcache.cpp bucketcache.h ddmsock2.cpp ddmsock2.h displaydriver.h listener.cpp listener.h
libddmsock2_la_LIBADD = $(NET_LIBS)

sysconf_DATA = ddmsock2.ini

##INCLUDES = -I$(top_srcdir)/boost -I$(top_srcdir)/render -I$(top_srcdir)/libddmsock -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libshaderexecenv
INCLUDES = -I$(top_srcdir)/boost -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libshaderexecenv -I$(top_srcdir)/thirdparty/tinyxml -I$(top_srcdir)/libdd2

AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@ -DTIXML_USE_STL
AM_CFLAGS=@AQSIS_CFLAGS@

EXTRA_DIST = libddmsock.dsp

CLEANFILES = ddmsock2.ini

devicelocal = $(top_builddir)/libddmsock2/ddmsock2.ini

ddmsock2.ini: Makefile.am
	cat /dev/null > $(devicelocal)
	echo "file	@bindir@/aqsis_display@EXEEXT@" >> $(devicelocal)
	echo "tiff	@bindir@/aqsis_display@EXEEXT@" >> $(devicelocal)
	echo "framebuffer	@bindir@/aqsis_display@EXEEXT@" >> $(devicelocal)	
	echo "zframebuffer	@bindir@/aqsis_display@EXEEXT@" >> $(devicelocal)
	echo "zfile	@bindir@/aqsis_display@EXEEXT@" >> $(devicelocal)
	echo "shadow	@bindir@/aqsis_display@EXEEXT@" >> $(devicelocal)
