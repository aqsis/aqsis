AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@ @AQSIS_OPENEXR_CFLAGS@ -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix
AM_CFLAGS=@AQSIS_CFLAGS@


pkglib_LTLIBRARIES = libexr2tif.la 

libexr2tif_la_SOURCES = exr2tif.cpp
libexr2tif_la_LIBADD = ../common/libcommon.la @AQSIS_OPENEXR_LIBS@ -lm -ltiff
