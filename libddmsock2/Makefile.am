lib_LTLIBRARIES = libddmsock2.la

libddmsock2_la_SOURCES = bucketcache.cpp bucketcache.h ddmsock2.cpp ddmsock2.h displaydriver.h listener.cpp listener.h
libddmsock2_la_LIBADD = $(NET_LIBS)

sysconf_DATA = ddmsock.ini

##INCLUDES = -I$(top_srcdir)/boost -I$(top_srcdir)/render -I$(top_srcdir)/libddmsock -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libshaderexecenv
INCLUDES = -I$(top_srcdir)/boost -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libshaderexecenv -I$(top_srcdir)/thirdparty/tinyxml -I$(top_srcdir)/libdd2

AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@ -DTIXML_USE_STL
AM_CFLAGS=@AQSIS_CFLAGS@

EXTRA_DIST = libddmsock.dsp

CLEANFILES = ddmsock.ini

devicelocal = $(top_builddir)/libddmsock/ddmsock.ini

ddmsock.ini:
	cat /dev/null > $(devicelocal)
	echo "file	@bindir@/filebuffer@EXEEXT@" >> $(devicelocal)
	echo "tiff	@bindir@/filebuffer@EXEEXT@" >> $(devicelocal)
	echo "framebuffer	@bindir@/aqsis_framebuffer_glut@EXEEXT@" >> $(devicelocal)	
	echo "zframebuffer	@bindir@/aqsis_framebuffer_glut_z@EXEEXT@" >> $(devicelocal)
	echo "zfile	@bindir@/shadowmap@EXEEXT@" >> $(devicelocal)
	echo "shadow	@bindir@/shadowmap@EXEEXT@" >> $(devicelocal)
