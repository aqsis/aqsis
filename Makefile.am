SUBDIRS = all images libaqsistypes libcodegenvm libdd libddmsimple libddmsock filebuffer framebuffer framebuffer_glut framebuffer_glut_z librib2 librib2ri librib2stream libslparse render ribfiles shadowmap slpp texer aqsl shaders aqsis
 
EXTRA_DIST = Doxyfile License.txt Renderer.dsw bootstrap prep_install.bat ribber.cfg 

test:
	cd aqsl/tests; $(MAKE) $(AM_MAKEFLAGS) test
	cd librib2/tests; $(MAKE) $(AM_MAKEFLAGS) test
	cd aqsis; $(MAKE) $(AM_MAKEFLAGS) test

.PHONY: test

