.. include:: common.rst

.. _features:

Features
--------

RenderMan
    |Aqsis| adheres to the RenderMan standard created by Pixar. As such it can interoperate well with other tools that also support the standard. It also means that experience gained working with |Aqsis| will be transferrable to other RenderMan compliant solutions.

Programmable Shading Pipeline
    |Aqsis| supports the full gamut of programmable shading functionality specified by the RenderMan standard. This includes surface, light, displacement, volume and imager shading. This provides the user with unprecedented flexibility in producing the look of the final image.

High Quality Texture Sampling and Filtering
    |Aqsis| provides flexible and efficient methods to sample and filter image based textures. It natively uses tile based Tiff files to allow memory efficient, intelligent caching of image textures. This allows the use of large numbers of high resolution images for various rendering effects, again improving the artistic control offered to the user. 

High Level Primitive Support
	|Aqsis| uses the Reyes rendering approach, which means all primitives are broken down into sub-pixel micropolygons (MP's) during rendering. This means that |Aqsis| only needs the high level surface descriptions to render from, reducing the data passed to the renderer, while ensuring a perfectly smooth silhouette edge. Some other renderers would require, for instance NURBS geometry to be polygonised before rendering. |Aqsis| will render a NURBS surface directly, while ensuring that it is rendered at a sufficiently high rate to prevent artifacts on silhouette edges. Effectively, the Reyes approach provides automatic, adaptive subdivision at render time, ensuring that the surface is subdivided enough to produce an accurate representation of curved surfaces in areas that need it, while not over subdividing in areas that don't need it. Many polygon based renderers would require a high level curved surface to be pre-subdivided, meaning the surface is likely to be subdivided too much in some areas, or not enough in others, often a user choice, this results in large amounts of data being sent to the renderer in order to avoid silhouette artifacts.

Sub-Pixel Displacement
	The Reyes approach allows |Aqsis| to provide true displacements at the sub-pixel level. Where some other renderers would need densely subdivided geometry to be passed into the rendering pipeline to achieve similar functionality, |Aqsis| provides this for all primitive types, with no requirement to alter the geometry before it is passed to the renderer.

Motion Blur (MB)
	|Aqsis| supports multi-segment motion blur. Objects can be described by any number of keyframes during a single shutter period and |Aqsis| will properly interpolate those keyframes to provide a motion blurred representation of the moving object. Allowing a completely arbitrary number of segments, allows the user to more accurately motion blur such things as rapidly rotating objects which, due to the linear interpolation of segments, is difficult ot achieve with only a few segments.
	
Depth of Field (DoF)
	|Aqsis| is able to accurately blur elements in the scene to emulate the focal capabilities of a real camera. Unlike a post processed depth blur, render time depth blurring accurately captures the effect of otherwise hidden scene elements showing through highly out of focus parts of the scene, a feature not possible with post processed blur using a depth map.

Shadow Mapped Ambient Occlusion
	|Aqsis| supports a special type of shadow map that contains the shadow information from a number of points in a single map, allowing ambient lighting that incorporates shadowing. By generating shadow maps from a hemisphere (or sphere) of lightsources surrounding the scene, and combining them into one large depth map, |Aqsis| is able to determine how occluded any part of the scene is from the surrounding ambient light. This information can be used to enhance the effect of ambient lighting, providing less illumination in areas that would naturally receive less ambient light.

Arbitrary Output Variables (AOV)
	|Aqsis| is able to output multiple images from a single render pass, each containing different information. The images can contain any shader variable, including the standard built in variables, such as surface normal, texture coordinates, surface derivatives etc. Alternatively, it is entirely possible to define new output variables of any supported RSL type to render any sort of surface information. These multiple passes can then be combined to produce various effects, such as cartoon rendering, or for complex post processing during compositing.

Subdivision Surfaces (SDS)
	|Aqsis| can render Catmull-Clark subdivision surfaces, subdividing to sub-pixel level at render time. No need to pass |Aqsis| a heavily subdivided mesh, |Aqsis| will subdivide as it renders, you only need to pass it the low poly control hull. As with [[doc:nurbs|NURBS]] surfaces, the subdivision of SDS primitives is adaptive, |Aqsis| will subdivide only as much as necessary to produce an accurate representation of the curved surface.


