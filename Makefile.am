SUBDIRS = all cribber filebuffer framebuffer images libaqsistypes libcodegenvm libdd libddmsimple libddmsock librib2 librib2ri librib2stream libslparse render ribfiles shaders shadowmap slcomp slpp texer aqsl aqsis
 
EXTRA_DIST = Doxyfile License.txt Readme.txt Renderer.dsw aqsis.ini bootstrap prep_install.bat ribber.cfg 

test:
	cd aqsl/tests; $(MAKE) $(AM_MAKEFLAGS) test
	cd librib2/tests; $(MAKE) $(AM_MAKEFLAGS) test
	cd aqsis; $(MAKE) $(AM_MAKEFLAGS) test

.PHONY: test

