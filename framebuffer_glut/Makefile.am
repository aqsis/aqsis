bin_PROGRAMS = aqsis_framebuffer_glut

aqsis_framebuffer_glut_SOURCES = framebuffer_glut.cpp
aqsis_framebuffer_glut_LDADD = $(top_builddir)/libdd/libdd.la $(top_builddir)/libaqsistypes/libaqsistypes.la -lglut -lGLU -lGL

if DEBUG
CXXFLAGS = -g -Wall
else
CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libdd -I$(top_srcdir)/libddmsock -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I@OPENGL_INC@

LDFLAGS = @LDFLAGS@ -L@OPENGL_LIB@

EXTRA_DIST = framebuffer_glut.dsp 
