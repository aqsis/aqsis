bin_PROGRAMS = aqsltell 

aqsltell_SOURCES = aqsltell.cpp
aqsltell_LDADD = $(top_builddir)/libslxargs/libslxargs.la $(top_builddir)/libshadervm/libshadervm.la $(top_builddir)/libshaderexecenv/libshaderexecenv.la $(top_builddir)/libaqsistypes/libaqsistypes.la @ARGPARSE_FLAGS@


INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/libshadervm -I$(top_srcdir)/libshaderexecenv -I$(top_srcdir)/libslxargs @ARGPARSE_INC@

EXTRA_DIST = aqsltell.dsp

