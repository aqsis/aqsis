---
layout: documentation
title: "Shading Language"
category: ri_standard
order: 30
---


Aqsis supports shaders written in the RenderMan Shading Language, RSL or simply
SL for short.  Shaders written for most other RenderMan implementations are
compatible with Aqsis, and vice versa.  Shaders written in SL are compiled by
''aqsl'' to an intermediate text-based format for efficient shader execution.
The compiled shader format is unique to Aqsis, and not compatible with other
renderers.

RSL Standard Conformance
------------------------

Aqsis supports the following shader types defined in the RenderMan standard:

  * Surface shaders
  * Displacement shaders
  * Light source shaders
  * Volume shaders
  * Imager shaders

All standard language constructs and RSL built-in functions are supported.
Shaders written for other RenderMan-compliant renderers should compile and work
with Aqsis. The RSL function trace() is formally allowed in shaders but always
returns black, because Aqsis is a strict Reyes renderer and does not (yet)
incorporate a ray tracing subsystem.

RSL Extensions
--------------

As an extension inspired by NVidia's abandoned renderer Gelato, Aqsis also
supports layered shaders, which enables the use of several separate shaders of
the same type to compose a desired effect. Shaders are connected by using output
variables from one as input variables to the next, in a flexible manner that
allows shaders to work together even if they were not originally designed for
that purpose.

The syntax of layered shaders is described in the article
[[guide:layered_shaders|Layered Shaders]].
