MAINDIRS = all images libargparse libaqsistypes libcodegenvm libdd libddmsimple libddmsock filebuffer librib2 librib2ri librib2stream libslparse libshadervm libshaderexecenv render ribfiles shadowmap slpp aqsl shaders libri2rib libslxargs teqser aqsis aqsltell dsotest

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

SUBDIRS = $(MAINDIRS) $(GLUTDIRS) $(PLUGINSDIRS)

EXTRA_DIST = Doxyfile Renderer.dsw bootstrap doxygen.dsp

.PHONY: test
test:
	cd aqsl/tests; $(MAKE) $(AM_MAKEFLAGS) test
	cd librib2/tests; $(MAKE) $(AM_MAKEFLAGS) test
	cd aqsis; $(MAKE) $(AM_MAKEFLAGS) test


