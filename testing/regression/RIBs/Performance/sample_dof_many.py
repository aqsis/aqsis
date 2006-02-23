from cgkit.ri import *

RiBegin (RI_NULL)
RiOption ("statistics","endofframe",3)

RiDisplay ("sample_dof_many.tif",RI_FILE,"rgba")
RiDepthOfField (1,80,16)
RiFormat (256,256,1)
RiShadingRate(256)
RiPixelSamples (4,4)
RiProjection (RI_ORTHOGRAPHIC)
RiFrameBegin (1)
RiWorldBegin ()
RiTranslate (0,0,5)
RiTransformBegin ()
RiScale (.5, .5, .5)
for i in xrange (1,25):
    RiTransformBegin ()
    RiTranslate (0,0,i)
    RiPatch(RI_BILINEAR, P=[-1,-1,0, -1,1,0, 1,-1,0, 1,1,0])
    RiTransformEnd ()
RiTransformEnd ()
RiWorldEnd ()

RiEnd
