shadersdir = $(pkgdatadir)/shaders

shader_headers = \
	DPProctext.h \
	bake.h \
	noises.h \
	patterns.h

shader_sources = \
	ambientlight.sl \
	background.sl \
	bluescreen.sl \
	borg_metal.sl \
	brickbump.sl \
	bumpy.sl \
	cellnoisetest.sl \
	constant.sl \
	curvetube.sl \
	debug.sl \
	dented.sl \
	depthcue.sl \
	distantlight.sl \
	dpbluemarble.sl \
	expensive.sl \
	exposure.sl \
	face_plastic.sl \
	fakesky.sl \
	fog.sl \
	gradient.sl \
	iramp.sl \
	matte.sl \
	metal.sl \
	paintedplastic.sl \
	plastic.sl \
	pointlight.sl \
	randgrid.sl \
	shadowdistant.sl \
	shadowpoint.sl \
	shadowspot.sl \
	shinymetal.sl \
	show_N.sl \
	show_st.sl \
	showuser.sl \
	spacetest.sl \
	spotlight.sl \
	sticky_texture.sl \
	uvtest.sl \
	wavy.sl

shader_binaries = \
	ambientlight.slx \
	background.slx \
	bluescreen.slx \
	borg_metal.slx \
	brickbump.slx \
	bumpy.slx \
	cellnoisetest.slx \
	constant.slx \
	curvetube.slx \
	debug.slx \
	dented.slx \
	depthcue.slx \
	distantlight.slx \
	dpbluemarble.slx \
	expensive.slx \
	exposure.slx \
	face_plastic.slx \
	fakesky.slx \
	fog.slx \
	gradient.slx \
	iramp.slx \
	matte.slx \
	metal.slx \
	paintedplastic.slx \
	plastic.slx \
	pointlight.slx \
	randgrid.slx \
	shadowdistant.slx \
	shadowpoint.slx \
	shadowspot.slx \
	shinymetal.slx \
	show_N.slx \
	show_st.slx \
	showuser.slx \
	spacetest.slx \
	spotlight.slx \
	sticky_texture.slx \
	uvtest.slx \
	wavy.slx

shaders_DATA = $(shader_headers) $(shader_sources) $(shader_binaries)

EXTRA_DIST = $(shader_headers) $(shader_sources) shaders.dsp shaders.vcproj

.sl.slx:
	$(top_builddir)/aqsl/aqsl $< -o $@

clean-local:
	rm -f *.slx
