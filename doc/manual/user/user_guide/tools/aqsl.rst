.. include:: ../../common.rst

.. index:: aqsl

.. _aqsl:

=====================
Shader Compiler: aqsl
=====================


Overview
--------

The aqsl ("axel") executable is the |Aqsis| shader compiler which processes 
shaders written in the |RenderMan| shading language into a form usable by
|Aqsis|.

One of the central features of the |RenderMan| interface is the ability to
describe materials, light sources and even the shapes of objects in terms of
separate, user-defined *shaders*.  A shader is a small program written in
the RenderMan Shading Language, |RSL| or SL for short.  The fact that shaders
are arbitrary programs gives an extreme versatility which has been a major
contributing factor in the popularity of RenderMan as a standard for producing
computer-generated images.

SL shaders are written as text files in a C-like syntax suitable for humans to
read and write; the shader compiler checks these programs for correctness and
translates them for use by the renderer.  The output ''.slx'' files produced
by aqsl are text, but in a regular syntax suitable for execution by the
renderer using a stack-based virtual machine.

Usage
-----

The typical way of invoking the shader compiler is by simply giving the
name of an SL source file as the only argument:
``aqsl myshader.sl```
This will compile the shader, saving the result in a file, *myshader.slx*.

.. index:: aqsl; command line options

Options
^^^^^^^

aqsl has several options, a summary of which can be obtained using ``aqsl -help``:

Usage: aqsl [options] [SL file]

  --o=string            Specify output filename
  --i=string            Set path for #include files.
  --I=string            Set path for #include files.
  --DSym=value          Define symbol Sym to have value *value* (default: 1).
  --USym                Undefine an initial symbol.
  --backend=string      Compiler backend (default slx).  Possibilities include "slx" or "dot":
                        slx - produce a compiled shader (in the aqsis shader VM stack language)
                        dot - make a graphviz visualization of the parse tree (useful for debugging only).
  -h, -help             Print this help and exit
  -version              Print version information and exit
  -nc, -nocolor         Disable colored output
  -d                    Dump sl data
  -v, --verbose=V       Set log output level
                        0 = errors
                        1 = warnings (default)
                        2 = information
                        3 = debug

All options can either begin with a single dash or two dashes and can appear anywhere on the command line. Most of the options are self explanatory, or adequately documented in the help output above, some require a little more explanation.

Compiler Backend
        aqsl is able to generate more than one type of output; the type of output desired is selected with the variable *backend_name*.  Currently available backends include *slx* and *dot*, of which *slx* is the default and produces programs in a format readable by the aqsis shader virtual machine.  *dot* is a debugging backend used to produce a graphviz graph of the internal abstract syntax tree generated from a shader (this isn't useful for the end user).
