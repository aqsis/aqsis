INCLUDES = -I$(top_srcdir)/libdd -I$(top_srcdir)/libddmsock -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix $(GL_CFLAGS)
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@


bin_PROGRAMS = aqsis_framebuffer_glut_z
aqsis_framebuffer_glut_z_SOURCES = framebuffer_glut_z.cpp
aqsis_framebuffer_glut_z_LDADD = $(top_builddir)/libdd/libdd.la $(top_builddir)/libaqsistypes/libaqsistypes.la $(LDFLAGS) $(GL_LIBS)

EXTRA_DIST = framebuffer_glut_z.dsp 
