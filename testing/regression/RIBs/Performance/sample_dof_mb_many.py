from cgkit.ri import *

RiBegin (RI_NULL)
RiOption ("statistics","endofframe",3)

RiDisplay ("sample_dof_many.tif",RI_FILE,"rgba")
RiDepthOfField (1,20,16)
RiFormat (512,512,1)
RiShadingRate(256)
RiPixelSamples (2,2)
RiProjection (RI_ORTHOGRAPHIC)
RiFrameBegin (1)
RiWorldBegin ()
RiTranslate (0,0,5)
RiTransformBegin ()
RiScale (.25, .25, .25)
for i in xrange (1,25):
    RiTransformBegin ()
    RiTranslate (0,0,i)
    RiAttributeBegin ()
    RiMotionBegin ([0,1])
    RiTranslate (-1,0,0)
    RiTranslate (1,0,0)
    RiMotionEnd ()
    RiPatch(RI_BILINEAR, P=[-1,-1,0, -1,1,0, 1,-1,0, 1,1,0])
    RiAttributeEnd ()
    RiTransformEnd ()
RiTransformEnd ()
RiWorldEnd ()

RiEnd
