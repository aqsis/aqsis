check_PROGRAMS = ribscanner ribprinter ribrenderer 

ribscanner_SOURCES = ribscanner.cpp
ribscanner_LDADD = $(top_builddir)/librib2/librib2.la $(top_builddir)/librib2stream/librib2stream.la

ribprinter_SOURCES = ribprinter.cpp
ribprinter_LDADD = $(top_builddir)/librib2/librib2.la $(top_builddir)/librib2stream/librib2stream.la

ribrenderer_SOURCES = ribrenderer.cpp
ribrenderer_LDADD = $(top_builddir)/librib2/librib2.la $(top_builddir)/librib2ri/librib2ri.la $(top_builddir)/render/libaqsis.la \
	$(top_builddir)/libddmsimple/libddmsimple.la $(top_builddir)/libaqsistypes/libaqsistypes.la -ltiff

CXXFLAGS = -g -Wall
INCLUDES = -I$(top_srcdir)/librib2 -I$(top_srcdir)/librib2stream -I$(top_srcdir)/librib2ri -I$(top_srcdir)/render -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix

EXTRA_DIST = test_001.in test_002.in test_003.in test_004.in cone.rib cylinder.rib nurbs.rib simple.rib sphere.rib teapot.rib torus.rib 

test-clean:
	rm -f *.tif
	rm -f *.z
	rm -f *.slx

test-shaders:
	for file in $$(find $(top_srcdir)/shaders -name "*.sl"); do $(top_builddir)/aqsl/aqsl $$file; done	

test-scripts:
	chmod a+x test_001
	chmod a+x test_002
	chmod a+x test_003
	chmod a+x test_004	

test: check test-clean test-shaders test-scripts
	$(SHELL) test_001
	$(SHELL) test_002
	$(SHELL) test_003
	$(SHELL) test_004

.PHONY: test-clean test-shaders test-scripts test
