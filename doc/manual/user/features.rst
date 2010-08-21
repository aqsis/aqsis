.. include:: common.rst

.. _features:

Features
--------

.. index:: renderman

RenderMan
    |Aqsis| adheres to the |RenderMan| standard created by Pixar. As such it can interoperate well with other tools that also support the standard. It also means that experience gained working with |Aqsis| will be transferrable to other |RenderMan| compliant solutions.

.. index:: programmable shading

Programmable Shading
    |Aqsis| supports the full gamut of programmable shading functionality specified by the |RenderMan| standard, including surface, light, displacement, volume and imager shaders. The |Aqsis| toolset includes a shader compiler that compiles |RSL| shaders for use in Aqsis, it compiles to an interpreted form targetting a custom virtual machine built into |Aqsis|.

.. index:: texturing, filtering, sampling

High Quality Texture Sampling and Filtering
    |Aqsis| provides flexible and efficient methods to sample and filter image based textures. It natively uses tile based Tiff files to allow memory efficient, intelligent caching of image textures. This allows the use of large numbers of high resolution images for various rendering effects, improving the artistic control offered to the user. 

.. index:: nurbs, high level primitives

High Level Primitive Support
	|Aqsis| uses the Reyes rendering approach, which means all primitives are split into tiny polygons, smaller than a pixel, during rendering. This approach allows |Aqsis| to render perfectly smooth primitives from high level descriptions, without the need to polygonise the geometry beforehand. Effectively, the Reyes approach provides automatic, adaptive subdivision at render time, ensuring that the surface is subdivided enough to produce an accurate representation of curved surfaces in areas that need it, while not producing unnecessarily high detail in areas that don't need it.

.. index:: displacement

Sub-Pixel Displacement
	The Reyes approach means that |Aqsis| can make fine adjustments to the surface as it is renderered, giving incredible control over the final look of geometry. These adjustments are applied using displacement shaders, which are free to modify the points of the surface almost at will. Alternative solutions such as bump and normal mapping simply provide the illusion of surface modification, but adjusting the lighting, the illusion is lost on silhouette edges, where the underlying smooth surface geometry remains evident.

.. index:: motion blur, mb

Motion Blur
	|Aqsis| supports multi-segment motion blur. Objects in the scene can be described by any number of keyframes during the shutter open/close period and |Aqsis| will properly interpolate those keyframes to provide a motion blurred representation of the moving and/or deforming object. Allowing a completely arbitrary number of segments, allows the user to more accurately motion blur such things as rapidly rotating objects which, due to the linear interpolation of segments, is difficult ot achieve with only a few segments.
	
.. index:: depth of field, dof

Depth of Field
	|Aqsis| is able to accurately blur elements in the scene to emulate the focal capabilities of a real camera. Unlike a post processed depth blur, render time depth blurring accurately captures the effect of otherwise hidden scene elements showing through highly out of focus parts of the scene, a feature not possible with post processed blur using a depth map.

.. index:: ambient occlusion, shadow mapped ambient occlusion

Shadow Mapped Ambient Occlusion
	|Aqsis| supports a special type of shadow map that contains the shadow information from a number of points in a single map, allowing ambient lighting that incorporates shadowing. By generating shadow maps from a hemisphere (or sphere) of lightsources surrounding the scene, and combining them into one large depth map, |Aqsis| is able to determine how occluded any part of the scene is from the surrounding ambient light. This information can be used to enhance the effect of ambient lighting, providing less illumination in areas that would naturally receive less ambient light.

.. index:: arbitrary output variable, aov

Arbitrary Output Variables (AOV)
	|Aqsis| is able to output multiple images from a single render pass, each containing different information. The images can contain any shader variable, including the standard built in variables, such as surface normal, texture coordinates, surface derivatives etc. Alternatively, it is entirely possible to define new output variables of any supported RSL type to render any sort of surface information. These multiple passes can then be combined to produce various effects, such as cartoon rendering, or for complex post processing during compositing.

.. index:: subdibivision surfaces, sds

Subdivision Surfaces (SDS)
	|Aqsis| can render Catmull-Clark subdivision surfaces, subdividing to sub-pixel level at render time. No need to pass |Aqsis| a heavily subdivided mesh, |Aqsis| will subdivide as it renders, you only need to pass it the low poly control hull. As with [[doc:nurbs|NURBS]] surfaces, the subdivision of SDS primitives is adaptive, |Aqsis| will subdivide only as much as necessary to produce an accurate representation of the curved surface.


