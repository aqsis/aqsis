SUBDIRS = . tests

EXTRA_DIST = parser.y scanner.l librib2.dsp

lib_LTLIBRARIES =  librib2.la

librib2_la_SOURCES = bdec.cpp bdec.h librib.cpp librib.h libribtypes.h y.tab.c y.tab.h lex.yy.c
librib2_la_LDFLAGS = -lz -rpath $(libdir)

include_HEADERS = librib.h

y.tab.c y.tab.h : $(srcdir)/parser.y $(srcdir)/libribtypes.h
	bison --defines -y $(srcdir)/parser.y

lex.yy.c : $(srcdir)/scanner.l y.tab.h $(srcdir)/libribtypes.h
	flex $(srcdir)/scanner.l

if DEBUG
AM_CXXFLAGS = -g -Wall 
AM_CFLAGS = -g -Wall -x c++
else
AM_CXXFLAGS = 
AM_CFLAGS = -x c++
endif

INCLUDES = -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix

