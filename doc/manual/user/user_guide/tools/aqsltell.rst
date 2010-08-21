.. include:: ../../common.rst

.. index:: aqsltell

.. _aqslell:

==========================
Shader Inspector: aqsltell
==========================


Overview
--------

Aqsltell ("Axle-tell") is a small tool to extract information about compiled shaders.  Passing the name of a compiled shader to aqsltell will present a formatted list of arguments, their types, and the default values. For example...

``aqsltell myshader``

Note that you do not pass the *.slx* extension, just the name of the shader, as you would use in the RIB stream. Aqsltell will search for the shader in the same way as aqsis would, using the shader searchpath. 

The structured information from aqsltell is intended to serve as input to third party tools, like a material editor or an exporter plugin for an animation tool or modeller. The tool can then present the user definable shader parameters to the user in a graphical interface so the user can set the values in a more friendly manner.

An example output from aqsltell for the standard shader *paintedplastic* is shown below for reference.

    surface "paintedplastic.slx"
        "Ka" "parameter uniform float[1]"
            Default value: 1
        "Kd" "parameter uniform float[1]"
            Default value: 0.5
        "Ks" "parameter uniform float[1]"
            Default value: 0.5
        "roughness" "parameter uniform float[1]"
            Default value: 0.1
        "specularcolor" "parameter uniform color[1]"
            Default value: "rgb" [1 1 1]
        "texturename" "parameter uniform string[1]"
            Default value: ""

Aqsltell is able to run the aqsis shadervm to determine information about the default values, where expressions are provided in the shader definition, the expression will be executed within a limited instance of the shadervm to determine the actual default value. This shadervm is limited in operation by the fact that it doesn't exist within a real rendering context, and has no access to surface information, so only certain shadervm operations can be validly executed.

Options
-------

A short help text listing all options can be displayed using the ''aqsltell -help'' option. All options can either begin with a single dash or two dashes and can appear anywhere on the command line.

Usage: aqsltell [shadername]

  -h, -help              Print this help and exit
  -version               Print version information and exit
  --shaders=string       Override the default shader searchpath(s) [C:\Program Files\Aqsis\shaders]

All options can either begin with a single dash or two dashes and can appear anywhere on the command line. 
