INCLUDES = -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix

lib_LTLIBRARIES = libri2rib.la

libri2rib_la_SOURCES = ascii.cpp ascii.h context.cpp context.h dictionary.cpp dictionary.h error.cpp error.h inlineparse.cpp inlineparse.h plstore.cpp plstore.h ri.cpp tokens.cpp

bin_PROGRAMS = ri2ribtest

CFLAGS = -lstdc++
ri2ribtest_SOURCES = ri2rib_test.c
ri2ribtest_LDADD = $(top_builddir)/libri2rib/libri2rib.la
