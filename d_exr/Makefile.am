EXTRA_DIST = dspyhlpr.c dspyhlpr.h tst.rib

AM_CXXFLAGS=-I/opt/local/include/OpenEXR
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@
AQSIS_OPENEXR_LIBS = 

pkglib_LTLIBRARIES = libexr.la

libexr_la_SOURCES = \
	d_exr.cpp 

libexr_la_LIBADD =-lIlmImf -lHalf -lIex -lImath -lm -ltiff 

INCLUDES = -I$(top_srcdir)/libddmanager -I$(top_srcdir)/render 

configdir = $(pkglibdir)

check-local:
	rm -f $(top_srcdir)/d_exr/libexr.so
	chmod a+w $(top_srcdir)/d_exr
	ln -s $(top_builddir)/d_exr/.libs/libexr.so $(top_srcdir)/d_exr/libexr.so
