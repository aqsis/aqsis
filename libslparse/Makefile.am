EXTRA_DIST = parser.y scanner.l libslparse.dsp

lib_LTLIBRARIES =  libslparse.la

libslparse_la_SOURCES = y.tab.c y.tab.h lex.yy.c funcdef.cpp funcdef.h ifuncdef.h iparsenode.h ivardef.h libslparse.cpp libslparse.h optimise.cpp parsenode.cpp parsenode.h typecheck.cpp vardef.cpp vardef.h
libslparse_la_LDFLAGS = -rpath $(libdir)

y.tab.c y.tab.h : $(srcdir)/parser.y 
	bison --defines -y $(srcdir)/parser.y

lex.yy.c : $(srcdir)/scanner.l y.tab.h 
	flex $(srcdir)/scanner.l

if DEBUG
AM_CXXFLAGS = -g -Wall
AM_CFLAGS = -g -Wall -x c++
else
AM_CXXFLAGS =
AM_CFLAGS = -x c++
endif

#Must disable optimisation here, because it takes far too much memory.
CXXFLAGS = @CXXFLAGS@ -O0
CFLAGS = @CFLAGS@ -O0
INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix 

