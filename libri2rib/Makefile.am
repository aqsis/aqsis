INCLUDES = -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


lib_LTLIBRARIES = libri2rib.la

libri2rib_la_SOURCES = ascii.cpp ascii.h binary.cpp binary.h context.cpp context.h dictionary.cpp dictionary.h error.cpp error.h inlineparse.cpp inlineparse.h options.h output.cpp output.h outstream.cpp outstream.h plstore.cpp plstore.h ri.cpp tokens.cpp ri.h
libri2rib_la_LDFLAGS = -lz -rpath $(libdir)

check_PROGRAMS = ri2ribtest
ri2ribtest_SOURCES = ri2ribtest.cpp
ri2ribtest_LDADD = $(top_builddir)/libri2rib/libri2rib.la

EXTRA_DIST = libri2rib.dsp COPYING_LGPL
