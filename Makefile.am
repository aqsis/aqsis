MAINDIRS = all images libargparse libaqsistypes libcodegenvm libdd libddmsimple libddmsock filebuffer librib2 librib2ri librib2stream libslparse libshadervm libshaderexecenv libraytrace render ribfiles shadowmap slpp aqsl shaders libri2rib libslxargs teqser aqsis aqsltell dsotest 

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

SUBDIRS = . $(MAINDIRS) $(GLUTDIRS) $(PLUGINSDIRS)

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
