bin_PROGRAMS = aqsis_framebuffer_glut_z

aqsis_framebuffer_glut_z_SOURCES = framebuffer_glut_z.cpp
aqsis_framebuffer_glut_z_LDADD = $(top_builddir)/libdd/libdd.la $(top_builddir)/libaqsistypes/libaqsistypes.la -lglut -lGLU -lGL

CXXFLAGS = -g -Wall
INCLUDES = -I$(top_srcdir)/libdd -I$(top_srcdir)/libddmsock -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix

