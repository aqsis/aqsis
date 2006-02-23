from cgkit.ri import *

RiBegin (RI_NULL)
RiOption ("statistics","endofframe",3)

RiDisplay ("sample_mb_move_many.tif",RI_FILE,"rgba")
RiFormat (128,128,1)
RiPixelSamples (3,3)
RiShadingRate (256)
RiProjection (RI_ORTHOGRAPHIC)
RiFrameBegin (1)
RiWorldBegin ()
RiTranslate (0,0,5)
RiTransformBegin ()
RiScale (.5, .5, .5)

for i in xrange (1,25):
    RiAttributeBegin ()
    RiTranslate (0,0,i*2)
    RiAttributeBegin ()
    RiMotionBegin( [0,1] )
    RiTranslate (-1,-1,0)
    RiTranslate (1,1,0)
    RiMotionEnd ()
    RiPatch(RI_BILINEAR, P=[-1,-1,0, -1,1,0, 1,-1,0, 1,1,0])
    RiAttributeEnd ()

    RiAttributeBegin ()
    RiTranslate (0,0,i*2+1)
    RiMotionBegin( [0,1] )
    RiTranslate (-1,1,0)
    RiTranslate (1,-1,0)
    RiMotionEnd ()
    RiPatch(RI_BILINEAR, P=[-1,-1,0, -1,1,0, 1,-1,0, 1,1,0])
    RiAttributeEnd ()
    RiAttributeEnd ()

RiTransformEnd ()
RiWorldEnd ()

RiEnd
