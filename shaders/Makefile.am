shader_include_files = $(wildcard $(srcdir)/*.h)
shader_source_files = $(wildcard $(srcdir)/*.sl)
shader_binary_files = $(addprefix $(top_builddir)/shaders/,$(notdir $(shader_source_files:.sl=.slx)))

.sl.slx:
	$(top_builddir)/aqsl/aqsl $< -o $@

shadersdir = $(pkgdatadir)/shaders

shaders_DATA = $(shader_include_files) $(shader_source_files) $(shader_binary_files)

all-local: $(shader_binary_files)

clean-local:
	rm -f *.slx

EXTRA_DIST = $(shader_include_files) $(shader_source_files)

