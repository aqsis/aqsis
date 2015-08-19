---
layout: documentation
title: "Texture Optimizer: teqser"
category: using_tools
order: 40
---


Overview
--------

**Teqser** (pronounced "Texer") is a tool that prepares texture images for use
by the renderer, a "texture compiler" if you like. Even though procedural
shading is a very powerful tool for defining surface appearance, there is still
a large number of applications where a texture image is the best solution.  A
texture image can be used by itself to define the colour, transparency or bumps
of a surface, but it can also be used as one part of a complicated procedural
shader.

Texture images need to be preprocessed for use by the renderer.  Regular
textures need to be prefiltered to prevent aliasing, and environment maps need
to be set up and assembled correctly.  The texture files for Aqsis are not
merely images, but slightly more complex data structures to give the renderer
better data to work with.

The input to the texture preprocessor is one or more image files in TIFF
format, specified on the command line. The output is a texture file that can be
used by an SL shader to perform texture lookups. The texture output format is
special to Aqsis and should not be used with any other renderer. In particular,
it is different from the texture file format in Pixar's PRMan.

The output file can have any name, but the recommended file extension is
`.tx`.

If you want to create a regular image texture, just invoke Teqser with the
input and output file names as options:

    teqser mytexture.tif mytexture.tx

There are several command-line options to Teqser to specify what kind of
texture you want to create. Please refer to the
[[doc:teqser_commands|appendix]] for details.


Options
-------

A short help text listing all options can be displayed using the `aqsis
-help` option. All options can either begin with a single dash or two dashes
and can appear anywhere on the command line.

    Usage: teqser [options] infile outfile
      -h, -help              Print this help and exit
      -version               Print version information and exit
      -v, -verbose=integer   Set log output level
                             0 = errors
                             1 = warnings (default)
                             2 = information
                             3 = debug
      -compression=string    [none|lzw|packbits|deflate] (default: none)
      -envcube px nx py ny pz nz
                             produce a cubeface environment map from 6 images.
      -envlatl               produce a latlong environment map from an image file.
      -shadow                produce a shadow map from a z file.
      -swrap=string          s wrap [black|periodic|clamp] (default: black)
      -smode=string          (equivalent to swrap for BMRT compatibility)
      -twrap=string          t wrap [black|periodic|clamp] (default: black)
      -tmode=string          (equivalent to twrap for BMRT compatibility)
      -wrap=string           wrap s&t [black|periodic|clamp]
      -mode=string           s (equivalent to wrap for BMRT compatibility)
      -filter=string         [box|bessel|catmull-rom|disk|gaussian|sinc|triangle|mitchell] (default: box)
      -fov(envcube)=float    [>=0.0f] (default: 90)
      -swidth, -sfilterwidth=float
                             s width [>0.0f] (default: 1)
      -twidth, -tfilterwidth=float
                             t width [>0.0f] (default: 1)
      -width, -filterwidth=float
                             width [>0.0f] set both swidth and twidth (default: -1)
      -quality=float         [>=1.0f && <= 100.0f] (default: 70)
      -bake=float            [>=2.0f && <= 2048.0f] (default: 128)
      -resize=string         [up|down|round|up-|down-|round-] (default: up)
                             Not used, for BMRT compatibility only!

-h / -help
  : Display the above help text.

-version
  : Display the version of the texture optimiser and exit.

-v / -verbose=integer
  : The verbosity level that determines how much text output Aqsis generates
    while it is running. See Table 1 for possible values.

-compression=string
  : Choose the compression type applied to the resulting image, choices are:

    * **none** - No compression is applied.
    * **lzw** - Use [Lempel-Ziv-Welch](http://en.wikipedia.org/wiki/Lzw)
      compression, due to patent restrictions, this may not be available on all
      platforms.
    * **packbits** - Use [packbits](http://en.wikipedia.org/wiki/Packbits)
      compression, a simple lossless run-length encoding scheme.
    * **deflate** - Use [deflate](http://en.wikipedia.org/wiki/Deflate)
      compression, a lossless scheme that combines LZ77 and Huffman coding.

-envcube px nx py ny pz nz
  : The 6 images passed form the faces of a cube, surrounding the scene. The
    cube can be used to simulate an environment for the purpose of reflection,
    lighting and other effects. The images passed are in the following order:

    1. The positive X side of the cube.
    2. The negative X side of the cube.
    3. The positive Y side of the cube.
    4. The negative Y side of the cube.
    5. The positive Z side of the cube.
    6. The negative X side of the cube.

-envlatl
  : Teqser will mark an image as an environment map of the surrounding scene
    spherically mapped to a single plane. Teqser makes no modification to the image
    itself, the image must already be mapped, it will filter and mip-map the image,
    and mark it as suitable for environment mapping.

-shadow
  : Teqser expects a depth file produced by aqsis using the `zfile` display
    driver, and will produce a shadow map. This display format is generally system
    specific, so the depth file should be produced on the same architecture and OS,
    preferably the same machine, as that on which teqser is being run, the same is
    not true of the shadow map, which is system agnostic.

-swrap=string
-smode=string
-twrap=string
-tmode=string
-wrap=string
-mode=string
  : Define the approach aqsis will take when dealing with out of range values
    in the `s`, `t` or both directions. Options are:

    * **black** - Values outside the 0...1 range in `s` will result in the colour
      black being returned.
    * **periodic** - Values outside the 0...1 range will wrap around to the
      opposite side of the range.
    * **clamp** - Values less than 0 will be sampled at 0, values greater than
      1, sampled at 1.

  : The `mode` variants are compatibility options, included to make
    transition from BMRT to Aqsis easier.

-filter=string
  : Define the filter that teqser will use to downsample the image when
    generating mip-maps. The details of the various filters are beyond the scope of
    this documentation.

-fov(envcube)=float
  : When rendering the images for use in a cubeface environment map, the field
    of view used to render each side is important during later sampling, this value
    defines the field of view as a half angle, as used in the RiProjection request.
    Ensure that this value exactly matches the value used when rendering the
    images, otherwise there will be noticeable artifacts at the seems.

-swidth, -sfilterwidth=float
-twidth, -tfilterwidth=float
-width, -filterwidth=float
  : Define the width of the filter used when down sampling an image during
    mip-mapping, in `s`, `t` or both directions.

-quality=float
  : When saving using jpeg compression, this value defines the jpeg quality
    value, 0 to 100.
  
-bake=float
  : When converting a bake file to a texture map, this value defines the
    resolution of the map in x and y.
  
-resize=string
  : Compatibility option, this value is ignored, provided only for
    compatibility with BMRT and PRman.


### Tables

|-------+--------------------------------|
| Level | Verbosity                      |
|-------|:-------------------------------|
|  0    | Only display errors            |
|  1    | Display warnings (default)     |
|  2    | Display informational messages |
|  3    | Display debug information      |
|-------+--------------------------------|

Table 1
