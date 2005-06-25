check_PROGRAMS = ribscanner ribprinter 
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


ribscanner_SOURCES = ribscanner.cpp
ribscanner_LDADD = $(top_builddir)/librib2/librib2.la $(top_builddir)/librib2stream/librib2stream.la $(top_builddir)/libaqsistypes/libaqsistypes.la @AQSIS_PLUGIN_LDFLAGS@


ribprinter_SOURCES = ribprinter.cpp
ribprinter_LDADD = $(top_builddir)/librib2/librib2.la $(top_builddir)/librib2stream/librib2stream.la $(top_builddir)/libaqsistypes/libaqsistypes.la @AQSIS_PLUGIN_LDFLAGS@

INCLUDES = -I$(top_srcdir)/librib2 -I$(top_srcdir)/librib2stream -I$(top_srcdir)/librib2ri -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix

