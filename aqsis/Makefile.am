bin_PROGRAMS = aqsis 

aqsis_SOURCES = aqsis.cpp
aqsis_LDADD = $(top_builddir)/librib2ri/librib2ri.la $(top_builddir)/librib2/librib2.la $(top_builddir)/render/libaqsis.la $(top_builddir)/libddmsimple/libddmsimple.la $(top_builddir)/libaqsistypes/libaqsistypes.la -ltiff -largparse

CXXFLAGS = -g -Wall
INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/render -I$(top_srcdir)/librib2 -I$(top_srcdir)/librib2ri 

test-clean:
	rm -f *.tif
	rm -f *.z
	rm -f *.slx

test-shaders:
	PATH=$$PATH:$(top_builddir)/aqsl; for file in $$(find $(top_srcdir) -name "*.sl" | sort); do $(top_builddir)/aqsl/aqsl $$file; done	

test: check test-clean test-shaders
	for file in $$(find $(top_srcdir)/ribfiles -name "*.rib" | sort); do echo "Rendering $$file ..."; $(top_builddir)/aqsis/aqsis $$file; done

.PHONY: test-clean test-shaders test

