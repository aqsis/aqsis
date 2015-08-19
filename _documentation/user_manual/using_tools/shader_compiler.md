---
layout: documentation
title: "Shader Compiler: aqsl"
category: using_tools
order: 20
---


Overview
--------

**aqsl** ("axel") is the Aqsis shader compiler.

One of the central features of the RenderMan Interface is the ability to
describe materials, light sources and even the shapes of objects in terms of
separate, user-defined *shaders*.  A shader is a small program written in the
RenderMan Shading Language, RSL or SL for short.  The fact that shaders are
arbitrary programs gives an extreme versatility which has been a major
contributing factor in the popularity of RenderMan as a standard for producing
computer-generated images.

SL shaders are written as text files in a C-like syntax suitable for humans to
read and write; the shader compiler checks these programs for correctness and
translates them for use by the renderer.  The output `.slx` files produced by
aqsl are text, but in a regular syntax suitable for execution by the renderer
using a stack-based virtual machine.

### Usage

The typical way of invoking the shader compiler is by simply giving the name of
an SL source file as the only argument:

    aqsl myshader.sl

This will compile the shader, saving the result in a file, `myshader.slx`.

Options 
-------

aqsl has several options, a summary of which can be obtained using `aqsl -help`:

    Usage: aqsl [options] <filename>
      -o %s                  specify output filename
      -i%s                   Set path for #include files.
      -I%s                   Set path for #include files.
      -DSym[=value]          define symbol <string> to have value <value> (default: 1).
      -USym                  Undefine an initial symbol.
      -backend %s            Compiler backend (default slx).  Possibilities include "slx" or "dot":
                             slx - produce a compiled shader (in the aqsis shader VM stack language)
                             dot - make a graphviz visualization of the parse tree (useful for debugging only).
      -h, -help              Print this help and exit
      -version               Print version information and exit
      -nc, -nocolor          Disable colored output
      -d                     dump sl data
      -v, -verbose=integer   Set log output level
                             0 = errors
                             1 = warnings (default)
                             2 = information
                             3 = debug

-o output_file_name
  : Specifies that the compiled shader should reside in `output_file_name`.

-i/-I path_to_includes
  : The RSL standard specifies that preprocessing be performed by a standard C
    preprocessor.  Therefore, shaders may make use of the `#include` preprocessor
    directive to pull content (such as additional function definitions) from
    external files.  These options add a directory to the search path for the
    `#include` directive.

-Dsome_symbol[=value]
  : Define the preprocessor symbol, `some_symbol`, and give it a value of
    `value`, or 1 if value is omitted.

-Usome_symbol
  : Undefine the preprocessor symbol `some_symbol`

-backend backend_name
  : aqsl is able to generate more than one type of output; the type of output
    desired is selected with the variable `backend_name`.  Currently available
    backends include `slx` and `dot`, of which `slx` is the default and
    produces programs in a format readable by the aqsis shader virtual machine.
    `dot` is a debugging backend used to produce a graphviz graph of the internal
    abstract syntax tree generated from a shader (this isn't useful for the end
    user).

-h/-help
  : Write the usage summary and options to stdout and exit.

-version
  : Print the current aqsl version and exit.

-nocolor
  : Disable color output in the error streams

-d
  : Dump the preprocessed shader code into a file for inspection.  The dump
    file name is derived from the shader file name by appending `.pp`.

-v level/-verbose=level
  : Set the verbosity of logging information printed to output streams: 0 -
    errors, 1 - warnings, 2 - extra information, 3 - verbose debugging information.

