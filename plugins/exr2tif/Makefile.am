EXTRA_DIST = 
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@ -I/opt/local/include/OpenEXR -I../../libaqsistypes
AM_CFLAGS=@AQSIS_CFLAGS@


pkglib_LTLIBRARIES = libexr2tif.la 

libexr2tif_la_SOURCES = exr2tif.cpp
libexr2tif_la_LIBADD = ../common/libcommon.la -L/opt/local/lib -lIlmImf -lHalf -lIex -lImath -lm -ltiff

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix
