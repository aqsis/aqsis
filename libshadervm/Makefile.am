EXTRA_DIST = libshadervm.dsp 

lib_LTLIBRARIES = libshadervm.la

libshadervm_la_SOURCES = dsoshadeops.cpp shadervm.cpp shadervm.h shadervariable.cpp shadervariable.h shaderstack.cpp shaderstack.h shadervm1.cpp shadervm2.cpp dsoshadeops.h 
libshadervm_la_LDFLAGS = -rpath $(libdir)

if DEBUG
AM_CXXFLAGS = -g -Wall
else
AM_CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/render -I$(top_srcdir)/libshaderexecenv


