lib_LTLIBRARIES = libddmsock.la
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


libddmsock_la_SOURCES = ddmsock.cpp ddmsock.h displaydriver.h

sysconf_DATA = ddmsock.ini

INCLUDES = -I$(top_srcdir)/render -I$(top_srcdir)/libddmsock -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libshaderexecenv

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
