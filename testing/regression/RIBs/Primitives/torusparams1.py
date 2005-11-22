#!/bin/sh 
""":" 
exec python $0 ${1+"$@"} 
""" 
# Test the parameters (phimin, phimax, thetamax) of a sphere.

import sys
from cgkit.cgtypes import *
from cgkit.ri import *
from cgkit.riutil import *

######################################################################
RiBegin(RI_NULL)

RiDisplay("torusparams1.tif", "file", "rgba", "string compression", "deflate")
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
phis = [0, -60, 60, 220, 360]
thetas  = [0, -60, 60, 220, 360]
for y in range(ncellsy):
	for x in range(ncellsx):
		phimin = phis[x]
		phimax = phis[x]
		theta = thetas[y]
		RiTransformBegin()
		RiTranslate((x-(ncellsx/2))*4, 0, (y-(ncellsy/2))*4)
		RiRotate(180,(0,0,1))
		RiScale(2.0,2.0,2.0)
		RiAttribute("identifier", "name", "Torus phimin:%f phimax:%f thetamax:%f"%(phimin,phimax,theta))
		if y & 1:
			RiTorus(0.6,0.2,phimin,180,theta)
		else:
			RiTorus(0.6,0.2,180,phimax,theta)
		RiuCoordSystem()
		RiTransformEnd()


RiWorldEnd()
RiEnd()
