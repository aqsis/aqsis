EXTRA_DIST = aqsis.dsp 

AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/render -I$(top_srcdir)/librib2 -I$(top_srcdir)/librib2ri -I$(top_srcdir)/libargparse

bin_PROGRAMS = aqsis 
aqsis_SOURCES = aqsis.cpp
aqsis_LDADD = $(top_builddir)/render/libaqsis.la $(top_builddir)/libraytrace/libraytrace.la $(top_builddir)/libargparse/libargparse.la

CONFIG_FILE = ./displays.ini
DISPLAY_LIB = @top_builddir@display/.libs/libdisplay.so
check-local:
	cat /dev/null > $(CONFIG_FILE)
	echo "file	$(DISPLAY_LIB)" >> $(CONFIG_FILE)
	echo "tiff	$(DISPLAY_LIB)" >> $(CONFIG_FILE)
	echo "framebuffer	$(DISPLAY_LIB)" >> $(CONFIG_FILE)
	echo "zframebuffer	$(DISPLAY_LIB)" >> $(CONFIG_FILE)
	echo "zfile	$(DISPLAY_LIB)" >> $(CONFIG_FILE)
	echo "shadow	$(DISPLAY_LIB)" >> $(CONFIG_FILE)
	for file in $(top_srcdir)/ribfiles/*.rib; do echo "Rendering $$file ..."; AQSIS_CONFIG_PATH="." $(top_builddir)/aqsis/aqsis -v3 -shaders=$(top_builddir)/shaders $$file; done

clean-local:
	rm -f $(CONFIG_FILE)
	rm -f *.map
	rm -f *.tif

