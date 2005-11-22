#!/bin/sh 
""":" 
exec python $0 ${1+"$@"} 
""" 
# Test the parameters (radius, zmin, zmax, thetamax) of a sphere.

import sys
from cgkit.cgtypes import *
from cgkit.ri import *
from cgkit.riutil import *

######################################################################
RiBegin(RI_NULL)

RiDisplay("sphereparams1.tif", "file", "rgba", "string compression", "deflate")
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
zmins = [-1.0, -0.5, 0.0, 0.5, 1.0]
zmaxs = [1.0, 0.5, 0.0, -0.5, -1.0]
thetas  = [0, -60, 60, 220, 360]
for y in range(ncellsy):
	for x in range(ncellsx):
		zmin = zmins[x]
		zmax = zmaxs[x]
		theta = thetas[y]
		RiTransformBegin()
		RiTranslate((x-(ncellsx/2))*4, 0, (y-(ncellsy/2))*4)
		RiRotate(180,(0,0,1))
		RiScale(2.0,2.0,2.0)
		RiAttribute("identifier", "name", "Sphere zmin:%f zmax:%f thetamax:%f"%(zmin,zmax,theta))
		if y & 1:
			RiSphere(1.0,zmin,1.0,theta)
		else:
			RiSphere(1.0,-1.0,zmax,theta)
		RiuCoordSystem()
		RiTransformEnd()


RiWorldEnd()
RiEnd()
