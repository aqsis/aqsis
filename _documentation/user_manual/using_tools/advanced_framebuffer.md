---
layout: documentation
title: "Advanced Framebuffer: piqsl"
category: using_tools
order: 60
---


Overview
--------

**piqsl** ("pixel") is the aqsis project's advanced framebuffer and image
viewer.  It's currently in an early stage of development, so feedback and
constructive criticism are most welcome.

The primary purpose of piqsl is to serve as the visible frontend to display
images as they are rendered (that is, to serve as a //framebuffer//).  Because
piqsl is multi-threaded and uses sockets for inter-process communication, it is
able to display rendering images from multiple concurrent aqsis renderer
processes.  The incoming images may be arranged into logical collections within
the interface, known as "books".

Apart from serving as a framebuffer, piqsl can load image files from disk (see
[[piqsl#supported image formats]]).

To render a file to piqsl, the RIB file needs to include an appropriate
"Display" command:

    Display "output.tif" "piqsl" "rgb"
    # Or, since piqsl is now the default aqsis framebuffer:
    Display "output.tif" "framebuffer" "rgb"

This connects the renderer to the piqsl display forwarding interface, which
automatically launches the piqsl executable.  For more details on the options
supported in the ''Display'' call above, see the [[display#piqsl|piqsl display
interface]].


The Main piqsl Window
---------------------

Images in piqsl are arranged into "books" - collections of images which are
associated together by the user.  A book or collection of books ("library") can
be saved to an XML "''.bks''" file for later retrieval.  Images in a book are
saved out as separate TIFF files; the ''.bks'' file simply stores the overall
structure of the collection.

The main piqsl window displays a single book at a time in the main tab.  The
book view shows a list of currently open images, along with some relevant
information about each image.


The Framebuffer Window
----------------------

Each book is associated with a separate framebuffer window which shows the
currently selected image.  The framebuffer image supports panning, zooming and
changing between various sub-images of the current image.

### Keyboard and mouse controls

**Panning** of an image is achieved by grabbing the image with the middle mouse
button and dragging.  **Zooming** by integer multiples of the base image
resolution may be achieved using the mouse ''scrollwheel'' or using ''KP+'' to
zoom in and ''KP-'' to zoom out (''KP'' signifies a key on the numeric keypad).
Zooming uses nearest-neighbour interpolation for simplicity and to minimize any
confusion about effects due to interpolation versus rendering artifacts.

Some image file formats such as TIFF can contain multiple sub-images in a
single file.  These **sub-images** can be accessed via the ''CTRL + KP+'' and
''CTRL + KP-'' shortcuts, which move to the next and previous sub-image in a
multi-image file respectively.

Additional hotkeys include ''h'' ("home") which centers the image on the
framebuffer window, and ''R'' which reloads the current image from file.


Options
-------

A short help text listing all options can be displayed using the ''piqsl
-help'' option:

    Usage: piqsl [options] [BKS file...] [Image file...]
      -i                     Specify the IP address to listen on (default: 127.0.0.1)
      -p                     Specify the TCP port to listen on (default: 49515)
      -h, -help              Print this help and exit
      -version               Print version information and exit
      -nc, -nocolor          Disable colored output
      -v, -verbose=integer   Set log output level
                             0 = errors
                             1 = warnings (default)
                             2 = information
                             3 = debug


Supported Image formats
-----------------------

Piqsl has support for reading the following image types:

  * TIFF - both tiled and scanline-based formats, various pixel formats, full
    support for multi-image files.
  * OpenEXR - most types, though no support for luminance-chroma or other image
    types with subsampled channels, and no support for multi-image (mipmapped)
    formats.

Piqsl can write images in the following formats:

  * TIFF - scanline-based, various pixel formats.
