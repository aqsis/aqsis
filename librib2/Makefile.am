SUBDIRS = . tests

EXTRA_DIST = parser.y scanner.l librib2.dsp

lib_LTLIBRARIES =  librib2.la

librib2_la_SOURCES = bdec.cpp bdec.h librib.cpp librib.h libribtypes.h parser.cpp parser.cpp.h scanner.cpp
librib2_la_LDFLAGS = -lz -rpath $(libdir)

include_HEADERS = librib.h

parser.cpp parser.cpp.h : $(srcdir)/parser.y $(srcdir)/libribtypes.h
	bison --defines -oparser.cpp $(srcdir)/parser.y

scanner.cpp : $(srcdir)/scanner.l parser.cpp.h $(srcdir)/libribtypes.h
	flex -oscanner.cpp $(srcdir)/scanner.l

if DEBUG
AM_CXXFLAGS = -g -Wall
else
AM_CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix

