---
layout: documentation
title: Using the Aqsis Tool Suite
category: part_i
order: 20
subcategory: using_tools
---

The aqsis tool suite consists of a collection of command line and graphical
tools. Final images are created from input scene description files, shaders and
textures by the renderer, aqsis. The other tools are used for manipulating the
input data to aqsis: Shader programs describing surface properties and
microgeometry should be compiled using the shader compiler, aqsl. The texture
optimizer teqser needs to be used for preprocessing image files before they are
used in a scene. miqser is a tool for for formatting and verifying scene
descriptions in the form of RIB streams. The small program aqsltell may be used
to inspect compiled shader files for information about the shader arguments.

There are two graphical tools, piqsl and eqsl. piqsl is the default framebuffer
device, used to interactively display the scene as it is rendered, or as
stand-alone image viewer. eqsl is a graphical interface aimed at making the
command line tools a little more friendly, especially to the casual user.
