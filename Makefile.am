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

devicedriver = $(bindir)/ddmsock.ini

install-data-local:
	cat /dev/null > $(devicedriver)
	echo "file	@bindir@/filebuffer@EXEEXT@" >> $(devicedriver)
	echo "tiff	@bindir@/filebuffer@EXEEXT@" >> $(devicedriver)
	echo "framebuffer	@bindir@/aqsis_framebuffer_glut@EXEEXT@" >> $(devicedriver)	
	echo "zframebuffer	@bindir@/aqsis_framebuffer_glut_z@EXEEXT@" >> $(devicedriver)
	echo "zfile	@bindir@/shadowmap@EXEEXT@" >> $(devicedriver)
	echo "shadow	@bindir@/shadowmap@EXEEXT@" >> $(devicedriver)


