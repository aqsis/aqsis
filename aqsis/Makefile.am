EXTRA_DIST = aqsis.dsp 

AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@ -DDEFAULT_RC_PATH=\"$(sysconfdir)/aqsisrc\" -DDEFAULT_SHADER_PATH=\".:$(pkgdatadir)/shaders\" -DDEFAULT_ARCHIVE_PATH=\".:$(pkgdatadir)/archives\" -DDEFAULT_TEXTURE_PATH=\".:$(pkgdatadir)/textures\" -DDEFAULT_DISPLAY_PATH=\"$(pkglibdir)\" -DDEFAULT_DSO_PATH=\"$(pkglibdir)\" -DDEFAULT_PROCEDURAL_PATH=\"$(pkglibdir)\"
AM_CFLAGS=@AQSIS_CFLAGS@

INCLUDES = -I$(top_srcdir)/libaqsistypes -I$(top_srcdir)/libaqsistypes/posix -I$(top_srcdir)/render -I$(top_srcdir)/librib2 -I$(top_srcdir)/librib2ri -I$(top_srcdir)/libargparse

bin_PROGRAMS = aqsis 
aqsis_SOURCES = aqsis.cpp
aqsis_LDADD = $(top_builddir)/render/libaqsis.la $(top_builddir)/libraytrace/libraytrace.la $(top_builddir)/libargparse/libargparse.la

check-local: clean-local
	for file in $(top_srcdir)/ribfiles/*.rib; do echo "Rendering $$file ..."; $(top_builddir)/aqsis/aqsis --displays="$(top_srcdir)/display" --shaders="$(top_srcdir)/shaders" -v3 $$file; done

clean-local:
	rm -f *.map
	rm -f *.tif

