## Process this file with automake to produce Makefile.in.

lib_LTLIBRARIES =  librib2.la

librib2_la_SOURCES = scanner.cpp parser.cpp librib.cpp librib.h libribtypes.h

pkginclude_HEADERS = librib.h

parser.cpp parser.cpp.h : $(srcdir)/parser.y $(srcdir)/libribtypes.h
	bison --defines -oparser.cpp $(srcdir)/parser.y

scanner.cpp : $(srcdir)/scanner.l parser.cpp.h $(srcdir)/libribtypes.h
	flex -oscanner.cpp $(srcdir)/scanner.l

EXTRA_DIST = parser.y scanner.l librib2.dsp

CXXFLAGS = -g -Wall
INCLUDES = -I$(top_srcdir)/render -I$(top_srcdir)/render/posix/intel

SUBDIRS = . tests
