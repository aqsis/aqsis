MAINDIRS = bin boost thirdparty all images libargparse libaqsistypes libcodegenvm librib2 librib2ri librib2stream libslparse libshadervm libshaderexecenv libraytrace ribfiles slpp aqsl shaders libri2rib libslxargs aqsltell dsotest display libddmanager render aqsis teqser man renderer.xcode

if PLUGINS
PLUGINSDIRS = plugins
else
PLUGINSDIRS = 
endif

SUBDIRS = . $(MAINDIRS) $(PLUGINSDIRS)

EXTRA_DIST = Doxyfile Renderer.dsw bootstrap doxygen.dsp Renderer.sln aqsis.spec

.PHONY: strace
strace:
	cd aqsis; $(MAKE) $(AM_MAKEFLAGS) strace

all-local: $(top_srcdir)/render/ri.inl $(top_srcdir)/render/ri_cache.inl $(top_srcdir)/render/ri_validate.inl

$(top_srcdir)/render/ri.inl: $(top_srcdir)/render/apiheader.xsl $(top_srcdir)/render/api.xml
	xsltproc -o $(top_srcdir)/render/ri.inl $(top_srcdir)/render/apiheader.xsl $(top_srcdir)/render/api.xml
	
$(top_srcdir)/render/ri_cache.inl: $(top_srcdir)/render/apicache.xsl $(top_srcdir)/render/api.xml
	xsltproc -o $(top_srcdir)/render/ri_cache.inl $(top_srcdir)/render/apicache.xsl $(top_srcdir)/render/api.xml

$(top_srcdir)/render/ri_validate.inl: $(top_srcdir)/render/apivalidate.xsl $(top_srcdir)/render/api.xml
	xsltproc -o $(top_srcdir)/render/ri_validate.inl $(top_srcdir)/render/apivalidate.xsl $(top_srcdir)/render/api.xml
