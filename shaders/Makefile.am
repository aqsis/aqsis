SHADER_INCLUDES = $(wildcard $(srcdir)/*.h)
SHADERS = $(wildcard $(srcdir)/*.sl)
COMPILED_SHADERS = $(addprefix $(top_builddir)/shaders/,$(notdir $(SHADERS:.sl=.slx)))

.sl.slx:
	$(top_builddir)/aqsl/aqsl $< -o $@

shadersdir = $(pkgdatadir)/shaders

shaders_DATA = $(SHADER_INCLUDES) $(SHADERS) $(COMPILED_SHADERS)

clean-local:
	rm -f *.slx

all-local: $(COMPILED_SHADERS)

EXTRA_DIST = $(SHADER_INCLUDES) $(SHADERS)

