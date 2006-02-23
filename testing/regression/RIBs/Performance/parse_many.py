from cgkit.ri import *

RiBegin (RI_NULL)
RiOption ("statistics","endofframe",3)

RiDisplay ("parse_many.tif",RI_FILE,"rgba")
RiFormat (1,1,1)
RiShadingRate(256)
RiPixelSamples (1,1)
RiProjection (RI_ORTHOGRAPHIC)
RiFrameBegin (1)
RiWorldBegin ()
RiTranslate (0,0,5)
RiTransformBegin ()
RiScale (1,1,1)
    
for i in xrange (0,4096*16):
    RiTranslate (0,0,1)
    RiSphere(1,1,-1,360)
RiTransformEnd ()
RiWorldEnd ()

RiEnd