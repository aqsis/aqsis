noinst_LTLIBRARIES = libplatform.la

AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@

libplatform_la_SOURCES = aqsis_compiler.h criticalsection.h semaphore.cpp semaphore.h share.h file_system.cpp
libplatform_la_LDFLAGS = -rpath $(libdir) -static

pkginclude_HEADERS = share.h

INCLUDES = -I$(top_srcdir)/libaqsistypes 

