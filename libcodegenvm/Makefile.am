EXTRA_DIST = libcodegenvm.dsp libcodegenvm.vcproj
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


lib_LTLIBRARIES =  libcodegenvm.la

libcodegenvm_la_SOURCES = codegenvm.cpp vmdatagather.cpp vmoutput.cpp codegenvm.h vmdatagather.h vmoutput.h 

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libslparse -I$(top_srcdir)/render

