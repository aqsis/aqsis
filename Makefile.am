SUBDIRS = all images libaqsistypes libcodegenvm libdd libddmsimple libddmsock filebuffer framebuffer_glut framebuffer_glut_z librib2 librib2ri librib2stream libslparse render ribfiles shadowmap slpp teqser aqsl shaders aqsis
 
EXTRA_DIST = Doxyfile License.txt Renderer.dsw bootstrap prep_install.bat ribber.cfg ddmsock.ini

.PHONY: test
test:
	cd aqsl/tests; $(MAKE) $(AM_MAKEFLAGS) test
	cd librib2/tests; $(MAKE) $(AM_MAKEFLAGS) test
	cd aqsis; $(MAKE) $(AM_MAKEFLAGS) test

devicedriver = $(bindir)/ddmsock.ini

install-data-local:
	cat /dev/null > $(devicedriver)
	echo "file	@bindir@/filebuffer" >> $(devicedriver)
	echo "tiff	@bindir@/filebuffer" >> $(devicedriver)
	echo "framebuffer	@bindir@/aqsis_framebuffer_glut" >> $(devicedriver)	
	echo "framebufferz	@bindir@/aqsis_framebuffer_glut_z" >> $(devicedriver)
	echo "zfile	@bindir@/shadowmap" >> $(devicedriver)
	echo "shadow	@bindir@/shadowmap" >> $(devicedriver)


