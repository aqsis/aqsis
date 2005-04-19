EXTRA_DIST = libshaderexecenv.dsp libshaderexecenv.vcproj
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


lib_LTLIBRARIES = libshaderexecenv.la

libshaderexecenv_la_SOURCES = shaderexecenv.cpp shaderexecenv.h shadeops.cpp

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/render -I$(top_srcdir)/libshadervm @AQSIS_BOOST_CPPFLAGS@


