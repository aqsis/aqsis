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

EXTRA_DIST = cone.rib cylinder.rib nurbs.rib simple.rib sphere.rib teapot.rib torus.rib 

test-clean:
	rm -f *.tif
	rm -f *.z
	rm -f *.slx

test-shaders:
	PATH=$$PATH:$(top_builddir)/aqsl; for file in $$(find $(top_srcdir) -name "*.sl"); do $(top_builddir)/aqsl/aqsl $$file; done	

test: check test-clean test-shaders
	for file in $(srcdir)/*.rib; do echo "Scanning $$file ..."; $(top_builddir)/librib2/tests/ribscanner < $$file; done
	for file in $(srcdir)/*.rib; do echo "Parsing $$file ..."; $(top_builddir)/librib2/tests/ribprinter $(top_srcdir)/ribber.cfg $$file; done
	for file in $(srcdir)/*.rib; do echo "Rendering $$file ..."; $(top_builddir)/librib2/tests/ribrenderer $(top_srcdir)/ribber.cfg $$file; done
	for file in $$(find $(top_srcdir)/content -name "*.rib"); do echo "Rendering $$file ..."; $(top_builddir)/librib2/tests/ribrenderer $(top_srcdir)/ribber.cfg $$file; done;

.PHONY: test-clean test-shaders test
