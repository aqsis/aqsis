SUBDIRS = libaqsistypes librib2 librib2stream librib2ri libslparse libcodegenvm libddmsimple libddmsock render aqsl aqsis libdd filebuffer shadowmap

EXTRA_DIST = ribber.cfg Renderer.dsw Doxyfile License.txt Readme.txt prep_install.bat aqsis.ini

test:
	cd aqsl/tests; $(MAKE) $(AM_MAKEFLAGS) test
	cd librib2/tests; $(MAKE) $(AM_MAKEFLAGS) test

dist-hook:
	mkdir $(distdir)/all
	cp -p all/*.dsp $(distdir)/all
	mkdir $(distdir)/shaders
	cp -p shaders/*.sl shaders/*.dsp $(distdir)/shaders
	mkdir $(distdir)/shaders/additional
	cp -p shaders/additional/*.sl shaders/additional/*.h $(distdir)/shaders/additional
	mkdir $(distdir)/slcomp
	cp -p slcomp/*.dsp slcomp/*.cpp $(distdir)/slcomp
	mkdir $(distdir)/slpp
	cp -p slpp/*.dsp slpp/*.C slpp/*.H slpp/*.DOC slpp/MAKEFILE $(distdir)/slpp
	mkdir $(distdir)/content
	cp -p content/*.rib content/*.dsp $(distdir)/content
	mkdir $(distdir)/content/Test
	cp -p content/Test/*.rib content/Test/*.sl content/Test/*.h content/Test/Image1.tif $(distdir)/content/Test
	mkdir $(distdir)/cribber
	cp -p cribber/*.cpp cribber/*.dsp $(distdir)/cribber
	mkdir $(distdir)/texer
	cp -p texer/*.cpp texer/*.dsp $(distdir)/texer
	mkdir $(distdir)/framebuffer
	mkdir $(distdir)/framebuffer/win32
	mkdir $(distdir)/framebuffer/win32/intel
	cp -p framebuffer/win32/intel/*.cpp framebuffer/win32/intel/*.h framebuffer/win32/intel/*.dsp framebuffer/win32/intel/*.rc framebuffer/win32/intel/*.ico $(distdir)/framebuffer/win32/intel

.PHONY: test

