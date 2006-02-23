from cgkit.ri import *

RiBegin (RI_NULL)
RiOption ("statistics","endofframe",3)

RiDisplay ("sample_comp_transp.tif",RI_FILE,"rgba")
RiFormat (256,256,1)
RiShadingRate(256)
RiPixelSamples (1,1)
RiProjection (RI_ORTHOGRAPHIC)
RiFrameBegin (1)
RiWorldBegin ()
RiTranslate (0,0,5)
RiTransformBegin ()
RiScale (1,1,1)
RiOpacity ([1.0/128,1.0/128,1.0/128])
    
for i in xrange (1,129):
    RiTransformBegin ()
    RiTranslate (0,0,i)
    RiPatch(RI_BILINEAR, P=[-1,-1,0, -1,1,0, 1,-1,0, 1,1,0])
    RiTransformEnd ()
RiTransformEnd ()
RiWorldEnd ()

RiEnd
