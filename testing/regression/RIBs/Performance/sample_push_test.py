from cgkit.ri import *

RiBegin (RI_NULL)
RiOption ("statistics","endofframe",3)

RiDisplay ("sample_push_test.tif",RI_FILE,"rgba")
RiFormat (256,256,1)
RiShadingRate(1)
RiPixelSamples (1,1)
RiProjection (RI_ORTHOGRAPHIC)
RiFrameBegin (1)
RiWorldBegin ()
RiTranslate (0,0,5)
RiTransformBegin ()
RiScale (1, 1, 1)
RiPatch(RI_BILINEAR, P=[-1,-1,0, -1,1,0, 1,-1,0, 1,1,0])
RiTransformEnd ()
RiWorldEnd ()

RiEnd
