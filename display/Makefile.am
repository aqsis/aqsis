bin_PROGRAMS = aqsis_display
aqsis_display_SOURCES = display.cpp
aqsis_display_LDADD = $(top_builddir)/boost/libs/thread/src/libthread.la $(top_builddir)/libargparse/libargparse.la $(top_builddir)/thirdparty/tinyxml/libtinyxml.la $(top_builddir)/libdd2/libdd2.la
aqsis_display_LDFLAGS = `fltk-config --ldflags` @TIFF_FLAGS@

INCLUDES = -I$(top_srcdir)/libdd2 -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/boost -I$(top_srcdir)/thirdparty/tinyxml -I$(top_srcdir)/libargparse `fltk-config --cflags` @TIFF_INC@

AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@ -DTIXML_USE_STL
AM_CFLAGS=@AQSIS_CFLAGS@

EXTRA_DIST = display.dsp

