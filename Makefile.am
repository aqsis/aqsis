MAINDIRS = boost thirdparty all images libargparse libaqsistypes libcodegenvm librib2 librib2ri librib2stream libslparse libshadervm libshaderexecenv libraytrace ribfiles slpp aqsl shaders libri2rib libslxargs aqsltell dsotest

RENDERDIRS = render aqsis

if PLUGINS
PLUGINSDIRS = plugins
else
PLUGINSDIRS = 
endif

if AQSIS_NEWDISPLAY
DISPLAYDIRS = libdd2 libddmsock2
if AQSIS_FLTK
FLTKDIRS = display
endif
else
DISPLAYDIRS = libdd libddmsimple libddmsock filebuffer shadowmap
TOOLSDIRS = teqser
if OPENGL
GLUTDIRS = framebuffer_glut framebuffer_glut_z
endif
endif

SUBDIRS = . $(MAINDIRS) $(DISPLAYDIRS) $(GLUTDIRS) $(PLUGINSDIRS) $(FLTKDIRS) $(RENDERDIRS) $(TOOLSDIRS)

EXTRA_DIST = Doxyfile Renderer.dsw bootstrap doxygen.dsp

.PHONY: test
test:
	cd aqsl/tests; $(MAKE) $(AM_MAKEFLAGS) test

all-local: $(top_srcdir)/render/ri.inl $(top_srcdir)/render/ri_cache.inl $(top_srcdir)/render/ri_validate.inl

$(top_srcdir)/render/ri.inl: $(top_srcdir)/render/apiheader.xsl $(top_srcdir)/render/api.xml
	xsltproc -o $(top_srcdir)/render/ri.inl $(top_srcdir)/render/apiheader.xsl $(top_srcdir)/render/api.xml
	
$(top_srcdir)/render/ri_cache.inl: $(top_srcdir)/render/apicache.xsl $(top_srcdir)/render/api.xml
	xsltproc -o $(top_srcdir)/render/ri_cache.inl $(top_srcdir)/render/apicache.xsl $(top_srcdir)/render/api.xml

$(top_srcdir)/render/ri_validate.inl: $(top_srcdir)/render/apivalidate.xsl $(top_srcdir)/render/api.xml
	xsltproc -o $(top_srcdir)/render/ri_validate.inl $(top_srcdir)/render/apivalidate.xsl $(top_srcdir)/render/api.xml

distclean-local:
	rm confdefs.h
