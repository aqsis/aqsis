---
layout: documentation
title: "RI Standard Conformance"
category: ri_standard
order: 10
---

Required Features
-----------------

Aqsis 1.2 and above implements all the required features of the RenderMan
Interface Specification:

  * Hierarchical graphics state
  * Orthographic and perspective viewing transforms
  * Depth-based hidden surface elimination
  * Pixel filtering and antialiasing
  * Gamma correction and dithering before quantization
  * RGB, A and Z output, in any combination and any resolution
  * All the standard primitives and associated variables
  * Programmable shading using the RenderMan Shading Language (SL)
  * Texture maps, environment maps and shadow maps
  * All the standard shaders (implemented entirely in SL)

It is worth mentioning that Aqsis allows //all// shader types, including
[[[doc:imagershaders|imager shaders]] to be written in SL, a feature that is
still unsupported by most commercial renderers, including Pixar's PRMan.

### Optional Features 

The set of required features in the specification is actually rather small, and
Aqsis, like most other RenderMan-compliant renderers, also supports many
additional features.  The specification lists the following optional advanced
capabilities. Some of them are implemented by Aqsis, and some are being
considered.

  * Solid Modeling - **supported**
  * Level of Detail - **supported**
  * Motion Blur - **supported**
  * Depth of Field - **supported**
  * Special Camera Projections - _not supported_
  * Displacements - **supported**
  * Spectral Colors - _not supported_
  * Volume Shading - **supported**
  * Ray Tracing - _not supported_
  * Global Illumination - _not supported_
  * Area Light Sources - _not supported_


Interface Extensions
--------------------

Aqsis supports a number of additional advanced features that are not mentioned
in the RenderMan Interface specification. Most of these features are modeled
after other renderers.  This section provides overviews of the additional
features in the style of the RISpec.

FIXME - write detailed specification (modelled on the RISpec) for each of the
interface extensions.

  * ambient occlusion
  * Arbitrary output variables (AOV) 
  * High dynamic range (HDRI) images 
  * Dynamic shadeops (DSO) 
  * Layered shaders 

### Ambient Occlusion

The ambient occlusion at a point on a surface is the amount of light which can
reach that point directly from infinity.  This provides a cheap kind of
approximate global illumination - deep holes in a surface are dark and edges
are highligthed when lit with an ambient occlusion light source.

The ambient occlusion may be computed by tracing rays in all directions from
the point and finding which ones go to infinity, or by rendering depth maps
from multiple points of view.  Conceptually, the depth map approach is
equivilant to having multiple ambient shadow lights lighting the scene from
many directions, and is the method supported by Aqsis.  

For a tutorial-style guide to using ambient occlusion, see [[guide:ao|Ambient
occlusion]].

    RtVoid RiMakeOcclusion ( RtInt npics, RtString picfiles[], RtString shadowfile, ...parameterlist... )

Create an ambient occlusion map from _npics_ depth files stored in the array
_picfiles_.  The depth files should result from rendering the scene from
multiple viewpoints.  Depending on scene setup, such viewpoints are typically
spaced evenly over a sphere or hemisphere centred on the object to be lit.

#### RIB BINDING 

    MakeOcclusion picfiles shadowfile ...parameterlist...

The number of input files is defined implicitly by the length of the
_picfiles_ array.


#### EXAMPLE 

    MakeOcclusion ["ao1.z" "ao2.z" "ao3.z" "ao4.z"] "ambient.map"



### Aqsis-specific geometry

The RenderMan interface defines a procedure **RiGeometry** for the purpose of
implementation specific geometry types. Aqsis uses this interface to provide
simple access to some of the more common geometry types familiar to the
computer graphics community. 

    RtVoid Rieometry ( RtToken type, ...parameterlist... )

This procedure provides a way for implementations to provide custom geometry
types.  Aqsis provides the following additional types.

  * **"teapot"** - A bicubic patch mesh representing the classic
    [Utah teapot](http://en.wikipedia.org/wiki/Utah_teapot).
  * **"bunny"** - A subdivision surface representing the
    [Standford Bunny](http://en.wikipedia.org/wiki/Stanford_Bunny).

#### RIB BINDING 

    Geometry type ...parameterlist...

#### EXAMPLE 

    RiGeometry ( "teapot", RI_NULL );

    RiGeometry ( "bunny", RI_NULL );


FIXME: document the following interface extensions -

### Conditional RIB 

    RtVoid RiIfBegin ( RtString condition )


    RtVoid RiElseIf ( RtString condition )


    RtVoid RiElse()


    RtVoid RiIfEnd()



### Layered Shaders 

    RiShaderLayer ( RtToken type, RtToken name, RtToken layername, ...parameterlist... )


    RiConnectShaderLayers ( RtToken type, RtToken layer1, RtToken variable1, RtToken layer2, RtToken variable2 )



### Appendix: Additional filter functions

    RiMitchellFilter

    RiDiskFilter

    RiBesselFilter



