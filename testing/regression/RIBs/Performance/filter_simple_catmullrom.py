from cgkit.ri import *

RiBegin (RI_NULL)
RiOption ("statistics","endofframe",3)

RiDisplay ("filter_simple_catmullrom.tif",RI_FILE,"rgba")
RiFormat (512,512,1)
RiShadingRate(256)
RiPixelFilter (RiCatmullRomFilter, 32,32)
RiPixelSamples (4,4)
RiProjection (RI_ORTHOGRAPHIC)
RiFrameBegin (1)
RiWorldBegin ()
RiRotate (1, 0,0,1)
RiTranslate (-.75,0,5)
RiTransformBegin ()
RiScale (.05, .66, .66)

for i in xrange (0,64):
	RiTranslate (0.5,0,0)
	RiPatch(RI_BILINEAR, P=[-.1,-1,0, -.1,1,0, .1,-1,0, .1,1,0])
RiTransformEnd ()
RiWorldEnd ()

RiEnd
