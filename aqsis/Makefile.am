bin_PROGRAMS = aqsis 
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@

aqsis_SOURCES = aqsis.cpp
aqsis_LDADD = $(top_builddir)/render/libaqsis.la $(top_builddir)/libraytrace/libraytrace.la $(top_builddir)/libargparse/libargparse.la

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/render -I$(top_srcdir)/librib2 -I$(top_srcdir)/librib2ri -I$(top_srcdir)/libargparse

check-local:
	cat /dev/null > ./ddmsock.ini
	echo "file	@top_builddir@filebuffer/filebuffer@EXEEXT@" >> ./ddmsock.ini
	echo "tiff	@top_builddir@filebuffer/filebuffer@EXEEXT@" >> ./ddmsock.ini
	echo "framebuffer	@top_builddir@framebuffer_glut/aqsis_framebuffer_glut@EXEEXT@" >> ./ddmsock.ini
	echo "zframebuffer	@top_builddir@framebuffer_glut_z/aqsis_framebuffer_glut_z@EXEEXT@" >> ./ddmsock.ini
	echo "zfile	@top_builddir@shadowmap/shadowmap@EXEEXT@" >> ./ddmsock.ini
	echo "shadow	@top_builddir@shadowmap/shadowmap@EXEEXT@" >> ./ddmsock.ini
	for file in $(top_srcdir)/ribfiles/*.rib; do echo "Rendering $$file ..."; AQSIS_CONFIG_PATH="./" $(top_builddir)/aqsis/aqsis -shaders=$(top_builddir)/shaders $$file; done

clean-local:
	rm -f ddmsock.ini
	rm -f *.map
	rm -f *.tif

EXTRA_DIST = aqsis.dsp 
