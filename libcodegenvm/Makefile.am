EXTRA_DIST = libcodegenvm.dsp

lib_LTLIBRARIES =  libcodegenvm.la

libcodegenvm_la_SOURCES = vmoutput.cpp vmoutput.h 
libcodegenvm_la_LDFLAGS = -rpath $(libdir)

if DEBUG
CXXFLAGS = -g -Wall
else
CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libslparse

