EXTRA_DIST = DPProctext.h ambientlight.sl borg_metal.sl bumpy.sl constant.sl debug.sl dented.sl depthcue.sl distantlight.sl dpbluemarble.sl fog.sl matte.sl metal.sl noises.h paintedplastic.sl patterns.h plastic.sl pointlight.sl randgrid.sl shaders.dsp shadowspot.sl shinymetal.sl spotlight.sl wavy.sl

shaderdir = $(prefix)/shaders

install-data-local:
	$(mkinstalldirs) $(shaderdir)
	$(INSTALL_DATA) $(srcdir)/*.h $(shaderdir)
	$(INSTALL_DATA) $(srcdir)/*.sl $(shaderdir)
	PATH=$$PATH:$(bindir); cd $(shaderdir); for file in $$(find . -name "*.sl"); do $(bindir)/aqsl $$file; done

