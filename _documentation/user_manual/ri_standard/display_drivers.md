---
layout: documentation
title: "Display Drivers"
category: ri_standard
order: 40
---


A **display driver** is a shared library which accepts the final pixel data from
the renderer.  Displays are loaded by the renderer at runtime and may perform
tasks such as saving the image to a file, or displaying it graphically to the
user.  It is possible to have several displays connected and recieving image
data at once.  Aqsis supports the common [[.:dev:display|display interface]]
defined by Pixar's PRMan, and supported by most RenderMan compliant renderers.
As such, many standard displays should be supported by Aqsis with no
modification. 

Aqsis is supplied with a number of display drivers providing output support for
various formats.  Graphical data is shown to the user through either the
[[display#framebuffer]], [[display#legacy framebuffer]], or
[[display#zframebuffer]].  Multiple image output formats are supported through
the [[display#File & TIFF]], [[display#XPM]], [[display#EXR]], and
[[display#BMP]] displays.  shadow maps may be generated with the
[[display#shadow]] or [[display#zfile]] displays.  Finally, there is a
[[display#debug]] display which is mainly useful to developers.

Displays may be selected using the `RiDisplay` command.  As a specific example
of display usage, placing the commands

    Display "output_file.tif" "tiff" "rgba" "string compression" "lzw"
    Display "+output_file.tif" "framebuffer" "rgb"

in the RIB input file would cause two displays to be connected to the renderer.
The first display is of type `"tiff"` and saves RGBA image data to the TIFF
file `"output_file.tif"` using lzw compression.  The `"string compression"
"lzw"` is an example of an optional _display parameter_ which can be used to
pass arbitrary data to a display from within the RIB file.  The second display
is of type `"framebuffer"` which causes aqsis to open a graphical interface
and display the image on screen as it is rendered.  The `"+"` which prefixes
the string `"+output_file.tif"` tells aqsis to _add_ the framebuffer to the
list of current displays; otherwise the first display command would be
overridden.  For further details, see the section describing `RiDisplay` in
the RiSpec.


Framebuffer
-----------

Since aqsis-1.4, the **framebuffer** device is an alias for the aqsis advanced
framebuffer, [[piqsl]].  The piqsl system comes in two parts:

  * A display device which recieves the data from aqsis and forwards it through
    a socket.
  * A multithreaded GUI frontend application which recieves that data and
    displays the render progress to the user in real time.

Since the main piqsl application runs in a seperate process and communicates
with the renderer using socket IO, multiple renders can be accumlated and
compared.  It also means that the framebuffer is _non-blocking_ which allows
aqsis to render multiple frames to the GUI and exit without user interaction.

A single piqsl frontend can recieve renders from multiple machines at the same
time, or from multiple processes on the same machine.  To redirect the output
to a piqsl frontend running on another machine, use the "host" and "port"
parameters.

#### Parameter summary

|---
| Name  | Valid values  | Default  ^ Description 
|---
| "int scanlineorder" | anything; not present in param list == off | off | read data from the renderer in scanline (not bucket) order |
| "string host" | any reachable host | "127.0.0.1" | host name which rendered data should be forwarded to |
| "string port" | port on host | "49515" | port on the remote host to which data should be sent |
|---



ZFramebuffer
------------

The **zframebuffer** display is used for showing depth data such as that used
for shadow mapping.  It uses the same code as the legacy framebuffer but does
some rescaling so that depth data can be viewed more easily.  Since piqsl
doesn't do any rescaling in aqsis-1.4, it is advisable to use the zframebuffer
for displaying depth data.

Parameters are the same as the legacy framebuffer.


Legacy Framebuffer
------------------

The **legacyframebuffer** display driver is the old, minimalistic framebuffer
from versions previous to aqsis-1.4.  Most users will probably want to switch to
using piqsl instead.


### Parameter summary

Parameters are the same as the file display device with one addition:

|---
| Name  | Valid values | Default  | Description 
|---
| "int scanlineorder" | anything; not present in param list == off | off | read data from the renderer in scanline (not bucket) order |
|---



File & TIFF
-----------

The **file** display driver saves the rendered image as a TIFF format file.  It
is capable of saving various bit depths, as automatically determined by the
chosen quantization information defined in the RIB file.

### Parameter summary

|---
| Name | Valid values | Default | Description 
|---
| "string compression" | "none", "lzw", "deflate", "jpeg", "packbits" | "lzw" | specifies the compression scheme to use |
| "int quality" | 0 - 100 | 75 | jpeg compression quality for TIFF files |
| "string description" | any string |  | image description to be stored in the file |
| "string HostComputer" | any string | computer name | name of host computer to be stored in the image file    |
|---



XPM
---

The **xpm** display device saves the rendered image in the xpm file format.  It
has no special user parameters.


EXR
---

The **exr** display device saves image data in the OpenEXR high dynamic range
format (original code from ILM).  OpenEXR is specifically designed for
representing HDR data, which means that it has excellent storage efficiency.
This is because of support for both (a) compression schemes for floating point
data and (b) a special 16-bit "half" data type.  

The OpenEXR format is able to store arbitrary number of named channels of mixed
type. To support this feature, Aqsis allows multiple calls to Display to specify
the same target image (using the standarg '+' syntax to indicate additional
outputs).  Each output to the same image is stored in a separate layer, using
the standard OpenEXR layer/channel naming convention, put simply, a channel C in
layer L is called "L.C", layers can be nested to support sub-layers, i.e.
"light1.specular.R".  Layers and channels can be named using options to the
Display request. If a layer isn't explicitly named, it will be given a name of
"layer__n_" where _n_ is the index of the layer, channels are named "R",
"G", "B" & "A" by default.

    Declare "layername" "string"
    Declare "channelname0" "string"
    Declare "channelname1" "string"
    Declare "channelname2" "string"

    # Store color in a layer named "Color" with channel names "R", "G" & "B"
    Display "image.exr" "exr" "rgb" "layername" "Color"
    # Store normal in a layer named "Normal" with channel names "X", "Y" & "Z".
    Display "+image.exr" "exr" "N" "#layername" "Normal" "channelname0" "X" "channelname1" "Y" "channelname2" "Z"

When you use this display driver for RGBA or Z output, you should
turn RGBA and Z quantization off by adding the following lines to
your RIB file:

    Quantize "rgba" 0 0 0 0
    Quantize "z"    0 0 0 0

You can alternatively use the inline quantization options described
[[guide:aov|here]].

Like Pixar's Tiff driver, this display driver can output image channels other
than R, G, B and A; for details on RIB file and shader syntax, see the Renderman
Release Notes (New Display System, RGBAZ Output Images, Arbitrary Output
Variables).

This driver maps Renderman's output variables to image channels as follows:

|---
| Renderman output variable name | image channel name | image channel type 
|---
| "r" | "R" | half |
| "g" | "G" | half |
| "b" | "B" | half |
| "a" | "A" | half |
| "z" | "Z" | float |
| other variable name | same as output (see below) | preferred type |
|---


By default, the "preferred" channel type is half; the preferred type can be
changed by adding an "exrpixeltype" argument to the Display command in the RIB
file.  For example:

    Declare "exrpixeltype" "string"

    # Store point positions in float format
    Display "gnome.points.exr" "exr" "P" "exrpixeltype" "float"

The default compression method for the image's pixel data is defined in
ImfHeader.h.  You can select a different compression method by adding an
"exrcompression" argument to the Display command.  For example:

    Declare "exrcompression" "string"

    # Store RGBA using run-length encoding
    Display "gnome.rgba.exr" "exr" "rgba" "exrcompression" "rle"

### Parameter summary

|---
| Name | Valid values | Default | Description |
|---
| "string exrcompression" | "none", "rle", "zips", "zip", "piz", "piz12"  | "zip"  | specifies the compression scheme to use |
| "string exrpixeltype" | "float", "half" | "half" ("float" for z channels) | specify the data channel precision |
| "string layername" | Any string | "layer_*n* | specify the layer name |
| "string channelname*0-3*" | Any string | "R", "G", "B", "A" | specify channel names for channel indexes 0-3 |
|---


BMP
---

The **bmp** display device saves the rendered image in the windows bitmap "bmp"
file format.  It has no special user parameters.



Shadow
------

The **shadow** display device will save a shadow map that can then be directly
used in a `shadow` call within a `lightsource` shader. It can be configured
using the following parameters passed to the **RiDisplay** request.


|---
| Name | Valid values | Default | Description 
|---
| "float append" | 0 or 1 | 0 | Causes the image to be appended to rather than overwriting a shadow file |
|---


If a non-zero value is supplied for the "append" option, the shadow map will be
appended as an additional directory to an existing shadow map TIFF file.  This
type of combined shadow map is intended for use with ambient occlusion, see
FIXME for details of the `RiMakeOcclusion` request used to further prepare
such occlusion map data.



ZFile
-----

The **zfile** display device saves depth data in an extremely simple
platform-dependent file format.  Data saved as zfiles can later be processed by
the `RiMakeShadow` interface call to create shadow maps.

The zfile display has the same parameters as the "file" display.



Debug
-----

The **debug** display is intended as a debugging aide to support a completely
static build of Aqsis suitable for debugging in situation where dynamic linking
causes problems.  The intent is to behave as much like a real dd as possible,
without doing anything useful.
