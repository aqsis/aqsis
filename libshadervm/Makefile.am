EXTRA_DIST = libshadervm.dsp 
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


lib_LTLIBRARIES = libshadervm.la

libshadervm_la_SOURCES = dsoshadeops.cpp shadervm.cpp shadervm.h shadervariable.cpp shadervariable.h shaderstack.cpp shaderstack.h shadervm1.cpp shadervm2.cpp dsoshadeops.h 
libshadervm_la_LDFLAGS = -rpath $(libdir) 

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/render -I$(top_srcdir)/libshaderexecenv


