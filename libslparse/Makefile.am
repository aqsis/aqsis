## Process this file with automake to produce Makefile.in.

lib_LTLIBRARIES =  libslparse.la

libslparse_la_SOURCES = parser.cpp scanner.cpp funcdef.cpp funcdef.h ifuncdef.h iparsenode.h ivardef.h libslparse.cpp libslparse.h optimise.cpp parsenode.cpp parsenode.h typecheck.cpp vardef.cpp vardef.h

pkginclude_HEADERS = 

parser.cpp parser.cpp.h : $(srcdir)/parser.y 
	bison --defines -oparser.cpp $(srcdir)/parser.y

scanner.cpp : $(srcdir)/scanner.l parser.cpp.h 
	flex -oscanner.cpp $(srcdir)/scanner.l

EXTRA_DIST = parser.y scanner.l

CXXFLAGS = -g -Wall
INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix 

SUBDIRS = . 
