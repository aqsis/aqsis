---
layout: documentation
title: Display Drivers
category: part_ii 
order: 30
subcategory: display_driver_api
---

Overview
--------

A **display driver** is a shared library that gets loaded by the renderer at
runtime and receives the final rendered image in individual chunks (buckets or
scanlines). The display driver is responsible for either displaying the image to
the user, storing it on disk, or whatever else is required in the rendering
pipeline.

Displays are connected to the renderer via the `RiDisplay()` interface call or
the `Display` RIB request (see the RISpec for details).  Although the method
for connecting displays is well-defined by the standard, the display interface
itself isn't formally described.  Nevertheless, the "//Dspy//" interface used by
Pixar's PRMan provides a de-facto standard which is implemented in Aqsis and
most other RenderMan renderers.  Implementing the same interface means that
compiled display drivers should be sharable between the various RenderMan
renderers.

Aqsis is distributed with a number of standard display drivers that allow you to
save the image in various file formats or display it directly in a framebuffer.
However, it is also possible to write your own customized display drivers using
the interface described in this section.


### Display Functions

Each display driver following the Dspy standard must implement four mandatory
functions:

  * [DspyImageOpen](#dspyimageopen) is the function which sets up the display.  It
    is called before the rendering starts.
  * [DspyImageQuery](#dspyimagequery) allows Aqsis to ask the display for
    additional information about an image.
  * [DspyImageData](#dspyimagedata) is called to pass the image data from the
    renderer to the display.
  * [DspyImageClose](#dspyimageclose) is called when the entire image has been
    rendered so that the display driver may free its resources.

There is one optional function:
  * [[#DspyImageDelayClose|DspyImageDelayClose]] may be used by interactive display drivers so that they can remain open while the renderer shuts down.

All functions are explained in detail in the [[#API Reference]].


