MAINDIRS = all images libaqsistypes libcodegenvm libdd libddmsimple libddmsock filebuffer librib2 librib2ri librib2stream libslparse render ribfiles shadowmap slpp aqsl shaders libri2rib releasearchive libslxargs libshadervm libshaderexecenv teqser aqsis aqsltell 

if GLUT
GLUTDIRS = framebuffer_glut framebuffer_glut_z
else
GLUTDIRS = 
endif

if PLUGINS
PLUGINSDIRS = plugins
else
PLUGINSDIRS = 
endif

SUBDIRS = $(MAINDIRS) $(GLUTDIRS) $(PLUGINSDIRS)

EXTRA_DIST = Doxyfile Renderer.dsw bootstrap prep_install.bat ribber.cfg ddmsock.ini 

.PHONY: test
test:
	cd aqsl/tests; $(MAKE) $(AM_MAKEFLAGS) test
	cd librib2/tests; $(MAKE) $(AM_MAKEFLAGS) test
	cd aqsis; $(MAKE) $(AM_MAKEFLAGS) test

devicepath = $(sysconfdir)
devicedriver = $(devicepath)/ddmsock.ini
devicelocal = $(top_builddir)/libddmsock/ddmsock.ini

all-local:
	cat /dev/null > $(devicelocal)
	echo "file	@bindir@/filebuffer@EXEEXT@" >> $(devicelocal)
	echo "tiff	@bindir@/filebuffer@EXEEXT@" >> $(devicelocal)
	echo "framebuffer	@bindir@/aqsis_framebuffer_glut@EXEEXT@" >> $(devicelocal)	
	echo "zframebuffer	@bindir@/aqsis_framebuffer_glut_z@EXEEXT@" >> $(devicelocal)
	echo "zfile	@bindir@/shadowmap@EXEEXT@" >> $(devicelocal)
	echo "shadow	@bindir@/shadowmap@EXEEXT@" >> $(devicelocal)

install-data-local:
	mkdir -p $(devicepath)
	cp $(devicelocal) $(devicedriver)

uninstall-local:
	rm $(devicedriver)
	rm $(devicelocal)


