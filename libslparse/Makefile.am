EXTRA_DIST = scanner.cxx parser.cxx parser.h libslparse.dsp
#Must disable optimisation here, because it takes far too much memory.
CXXFLAGS=
AM_CXXFLAGS=-O0
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=-O0


lib_LTLIBRARIES =  libslparse.la
AM_YFLAGS=-d

libslparse_la_SOURCES = parser.yxx scanner.lxx funcdef.cpp funcdef.h ifuncdef.h iparsenode.h ivardef.h libslparse.cpp libslparse.h optimise.cpp parsenode.cpp parsenode.h typecheck.cpp vardef.cpp vardef.h icodegen.h

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix

