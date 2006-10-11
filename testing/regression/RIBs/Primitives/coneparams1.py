#!/bin/sh 
""":" 
exec python $0 ${1+"$@"} 
""" 
# Test the parameters (height, thetamax) of a cone.

import sys
from cgkit.cgtypes import *
from cgkit.ri import *
from cgkit.riutil import *

######################################################################
RiBegin(RI_NULL)

RiDisplay("coneparams1.tif", "file", "rgba", "string compression", "lzw")
#RiDisplay("test.tif", "framebuffer", "rgba")
RiFormat(480,360,1)
RiPixelSamples(2,2)
RiShadingRate(1.0)

RiOption("statistics","endofframe",1)
RiOption("searchpath","shader",["../../shaders:&"])

RiTransformBegin()
RiLightSource("distantlight", to=(-0.8,-0.5,1))
RiTransformEnd()

RiProjection("perspective", fov=35)
RiConcatTransform(mat4().lookAt(vec3(0,25,20), vec3(0,0,2), vec3(0,0,1)).inverse())

RiWorldBegin()
RiSurface("matte")
RiSurface("show_st")

#RiuCoordSystem()
#RiuGrid()

ncellsx = 5
ncellsy = 5
heights = [-2.0, -1.0, 0.0, 1.0, 2.0]
thetas  = [0, -60, 60, 220, 360]
for y in range(ncellsy):
    for x in range(ncellsx):
#        tx = float(x)/(ncellsx-1)
#        ty = float(y)/(ncellsy-1)
#        height = -2*(1.0-tx) + 2.0*tx
#        theta = 0*(1.0-ty) + 360*ty
        height = heights[x]
        theta = thetas[y]
        RiTransformBegin()
        RiTranslate((x-(ncellsx/2))*4, 0, (y-(ncellsy/2))*4)
        RiRotate(180,(0,0,1))
        RiAttribute("identifier", "name", "Cone height:%f thetamax:%f"%(height,theta))
        RiCone(height,2,theta)
        RiuCoordSystem()
        RiTransformEnd()


RiWorldEnd()
RiEnd()
