bin_PROGRAMS = aqsltell 
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@ -DDEFAULT_SHADER_PATH=\".:$(pkgdatadir)/shaders\" 
AM_CFLAGS=@AQSIS_CFLAGS@


aqsltell_SOURCES = aqsltell.cpp
aqsltell_LDADD = $(top_builddir)/libslxargs/libslxargs.la $(top_builddir)/libshadervm/libshadervm.la $(top_builddir)/libshaderexecenv/libshaderexecenv.la $(top_builddir)/libaqsistypes/libaqsistypes.la $(top_builddir)/libargparse/libargparse.la @AQSIS_PLUGIN_LDFLAGS@


INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libshadervm -I$(top_srcdir)/libshaderexecenv -I$(top_srcdir)/libslxargs -I$(top_srcdir)/libargparse -I$(top_srcdir)/render

EXTRA_DIST = aqsltell.dsp aqsltell.vcproj

