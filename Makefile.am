MAINDIRS = boost all images libargparse libaqsistypes libcodegenvm libdd libdd2 libddmsimple libddmsock libddmsock2 filebuffer librib2 librib2ri librib2stream libslparse libshadervm libshaderexecenv libraytrace render ribfiles shadowmap slpp aqsl shaders libri2rib libslxargs teqser aqsis aqsltell dsotest thirdparty

if OPENGL
GLUTDIRS = framebuffer_glut framebuffer_glut_z
else
GLUTDIRS = 
endif

if PLUGINS
PLUGINSDIRS = plugins
else
PLUGINSDIRS = 
endif

if AQSIS_FLTK
FLTKDIRS = display
else
FLTKDIRS =
endif

SUBDIRS = . $(MAINDIRS) $(GLUTDIRS) $(PLUGINSDIRS) $(FLTKDIRS)

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
