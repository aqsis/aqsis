bin_PROGRAMS = aqsis 
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@

aqsis_SOURCES = aqsis.cpp
aqsis_LDADD = $(top_builddir)/render/libaqsis.la $(top_builddir)/libargparse/libargparse.la

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/render -I$(top_srcdir)/librib2 -I$(top_srcdir)/librib2ri -I$(top_srcdir)/libargparse

test-clean:
	rm -f *.tif
	rm -f *.z
	rm -f *.slx
	rm -f *.map

test-shaders:
	PATH=$$PATH:$(top_builddir)/aqsl; for file in $$(find $(top_srcdir) -name "*.sl" | sort); do $(top_builddir)/aqsl/aqsl $$file; done	

test: check test-clean test-shaders
	cat /dev/null > ./ddmsock.ini
	echo "file	@top_builddir@filebuffer/filebuffer@EXEEXT@" >> ./ddmsock.ini 
	echo "tiff	@top_builddir@filebuffer/filebuffer@EXEEXT@" >> ./ddmsock.ini
	echo "framebuffer	@top_builddir@framebuffer_glut/aqsis_framebuffer_glut@EXEEXT@" >> ./ddmsock.ini	
	echo "zframebuffer	@top_builddir@framebuffer_glut_z/aqsis_framebuffer_glut_z@EXEEXT@" >> ./ddmsock.ini
	echo "zfile	@top_builddir@shadowmap/shadowmap@EXEEXT@" >> ./ddmsock.ini
	echo "shadow	@top_builddir@shadowmap/shadowmap@EXEEXT@" >> ./ddmsock.ini
	for file in $$(find $(top_srcdir)/ribfiles -name "*.rib" | sort); do echo "Rendering $$file ..."; export AQSIS_CONFIG_PATH=./; $(top_builddir)/aqsis/aqsis $$file; done
	rm ./ddmsock.ini

.PHONY: test-clean test-shaders test

EXTRA_DIST = aqsis.dsp 
