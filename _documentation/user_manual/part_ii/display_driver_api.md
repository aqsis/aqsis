---
layout: documentation
title: API Reference
category: display_driver_api 
order: 20
---


API Reference
-------------

Aqsis expects a display to implement the standard display interface as in PRMan
and other renderers.  This means implementing the four functions
`DspyImageOpen`, `DspyImageQuery`, `DspyImageData` and `DspyImageClose`.
The function prototypes are defined in the header `<ndspy.h>`.

All functions in the display interface return one of the possible
[[#PtDspyError|error codes]], which will normally be `PkDspyErrorNone` when
everything is ok.  The first parameter to the display interface functions is an
image handle of type [[#PtDspyImageHandle]].  The handle should be used by the
display driver to store internal data associated with the display.  Any handle
stored here by `DspyImageOpen` will be passed on to the other functions
without being modified by the renderer.



### DspyImageOpen

    PtDspyError DspyImageOpen(PtDspyImageHandle*   image,
                              const char*          drivername,
                              const char*          filename,
                              int                  width,
                              int                  height,
                              int                  paramCount,
                              const UserParameter* parameters,
                              int                  formatCount,
                              PtDspyDevFormat*     format,
                              PtFlagStuff*         flagstuff);

`DspyImageOpen` is called by the renderer to open and initialize a display.
Typically this might involve allocating a memory buffer large enough to hold the
image, opening a file or initializing a GUI to display the data directly.


#### Parameters

  * `image` - a pointer to a handle for any internal data structures allocated
    by the display.  This handle should be set by `DspyImageOpen` and will be
    stored by the renderer without modification to be passed on to the other
    display functions.
  * `drivername` - the name of the display driver as it was specified in the
    `RiDisplay()` call.
  * `filename` - the output file name as given in the `RiDisplay()` call.
  * `width/height` - the image resolution in pixel. When the image is cropped
    this is the cropped size. Either value may be 0 in which case the driver has
    to choose an appropriate default value.
  * `paramCount` - the number of parameters stored in the array
    `parameters`.
  * `parameters` - Parameters to the `RiDisplay()` interface call are passed
    on to the display inside this array.  For each parameter there is a
    [UserParameter](#userparameter) struct that contains the name/value pair.
    Important note: These structs are only valid within this function. If you
    need a value in one of the other functions you must copy the value! In
    addition to the user-defined parameters, there are a number of parameters
    the renderer always sends to the driver (see table below).
  * `formatCount` - the number of items in the array `format`, that is, the
    number of channels of pixel data in the image.
  * `format` - For each output channel, there is a
    [PtDspyDevFormat](#ptdspydevformat) struct describing the type of the data
    that gets passed into the driver. The driver may rearrange this array or
    change the data type to get the channels delivered in a different order or
    as a different type or in a different byte order. If you want to remove a
    channel, set its type to `PkDspyNone`.  Important notes: 1) When you
    reorder the array you must ensure that the names still point to the original
    strings that were passed in! 2) Just like the `parameters` array the
    format structs are only valid inside this call. Do not try to access the
    values again in one of the other functions.
  * `flagstuff` - The display may change the value of these flags to modify
    aspects of the incoming pixel data, such as to request that the renderer
    send scanline order.  See [PtFlagStuff](#ptflagstuff) for possible values.

In addition to any user parameters passed to the driver, the array
`parameters` will always contain the following system-defined parameters:

|--------------------|---------------|-----------------------------------------------------------------------|
| name               | vtype[vcount] | Description                                                           |
|--------------------|---------------|-----------------------------------------------------------------------|
| "NP"               | f[16]         | World to screen space matrix                                          |
| "Nl"               | f[16]         | World to camera space matrix                                          |
| "near"             | f[1]          | Near clipping plane                                                   |
| "far"              | f[1]          | Far clipping plane                                                    |
| "origin"           | i[2]          | Origin of the crop window within the entire image                     |
| "OriginalSize"     | i[2]          | Original pixel size of the entire image (not just the cropped window) |
| "PixelAspectRatio" | f[1]          | The pixel aspect ration                                               |
| "Software"         | s[1]          | The name of the renderer                                              |
| "HostComputer"     | s[1]          | The network name of the computer that the image is rendered on        |
|--------------------|---------------|-----------------------------------------------------------------------|



#### Types

##### UserParameter

    typedef struct
    {
        RtToken   name;
        char      vtype;
        char      vcount;
        RtPointer value;
        int       nbytes;
    }
    UserParameter;

  * `name` - the name of the parameter
  * `vtype` - the type of the parameter value. This can be either `"i"` for
    integers of type `RtInt`, `"f"` for floats of type `RtFloat` or
    `"s"` for strings of type `char*`.
  * `vcount` - this is the array size. If the value is a scalar, `vcount` is
    1.
  * `value` - this is a pointer that points to the value(s).  The display
    should cast this pointer to the appropriate type to access the contained
    data.
  * `nbytes` - the total number of bytes used by the entire value


##### PtDspyDevFormat

    typedef struct
    {
        char *name;
        unsigned type;
    }
    PtDspyDevFormat;

  * `name` - the name of the output channel.  Standard channels include `"r"`,
    `"g"`, `"b"`, `"a"`, `"z"`, but `name` may take other values when using AOV.
  * `type` - one of the possible [[#Pixel types|PkDspy constants]] describing
    the channel bitwidth and type. The type can be combined (using the bitwise
    OR operator) with the following flags to determine the byte order of a
    single value.  Obviously this is only meaningful for types that are longer
    than a single byte.
    * `PkDspyByteOrderHiLo`: Force big endian format (most significant bytes
      first)
    * `PkDspyByteOrderLoHi`: Force little endian format (least significant bytes
      first)
    * `PkDspyByteOrderNative`: Use whatever format is native on the current
      machine (this is the default)


##### Pixel types

A set of integer constants is defined to specify the output pixel format type.
All such constants start with the prefix `Pk`.  Associated with these is a set
of typedefs starting with the prefix `Pt`.  Pixel formats are defined both for
floating point and signed/unsigned integers of different bit-widths. The value
`PkDspyNone` may be set by the display to remove a channel from the data sent to
the driver.  For example, if the display can only handle rgb images, but the
renderer is going to create rgba data, the display should set the type for the
`"a"` field to `PtDspyNone`.

|------------------|------------------|
| Format constant  | typedef          |
|------------------|------------------|
| PkDspyNone       | -                | 
| PkDspyFloat32    | PtDspyFloat32    | 
| PkDspyUnsigned32 | PtDspyUnsigned32 | 
| PkDspySigned32   | PtDspySigned32   | 
| PkDspyUnsigned16 | PtDspyUnsigned16 | 
| PkDspySigned16   | PtDspySigned16   | 
| PkDspyUnsigned8  | PtDspyUnsigned8  | 
| PkDspySigned8    | PtDspySigned8    | 
|------------------|------------------|


##### PtFlagStuff

    typedef struct
    {
        int flags;
    }
    PtFlagStuff;

`flags` is a bitwise or of the following constants:

  * `PkDspyFlagsWantsScanLineOrder` - Send the data in scanline order rather
    than as buckets.  Scanline order starts from the top-left of the image,
    working downward line by line.  Note that the display shouldn't assume that
    a whole scanline will arrive at once.
  * `PkDspyFlagsWantsEmptyBuckets` - Send pixel data even for buckets which
    contain no primitives.
  * `PkDspyFlagsWantsNullEmptyBuckets` - Generate calls to `DspyImageData` for
    empty buckets, but set the associated `data` pointer to null.



-----



### DspyImageQuery 

    PtDspyError DspyImageQuery(PtDspyImageHandle image,
                               PtDspyQueryType   type,
                               size_t            size,
                               void*             data);

This function is used by the renderer to obtain further information about the
image or the display driver itself. Aqsis passes in the image handle that was
initialised in the open function (or `NULL` if the query does not relate to a
particular image), a query type that determines what kind of information is
requested and a data block that has to be filled in by the driver with
appropriate values.

  * `image` - this is either the image handle that was returned in
    [[#DspyImageOpen|DspyImageOpen]] or a `NULL` pointer.
  * `type` - determines the type of information that is requested. The possible
    types are defined in the enum [[#PtDspyQueryType|PtDspyQueryType]].
  * `size` - the size in bytes of the memory that `data` points to. Make sure
    not to write beyond this size!
  * `data` - an allocated memory buffer of `size` bytes that has to be filled in
    by the display driver. Depending on the query type this points to an
    appropriate info struct (see below).

The following requests may be made by the renderer:

  * **PkSizeQuery**: The renderer wants to know the pixel size of an image and
    the pixel aspect ratio (when `image` is NULL, default values have to be
    returned). The values have to be filled into a
    [[#PtDspySizeInfo|PtDspySizeInfo]] structure and copied to `data`.

  * **PkOverwriteQuery**: This is used to query the driver whether it will
    overwrite an image file or not. For this query, `data` should receive a
    [[#PtDspyOverwriteInfo|PtDspyOverwriteInfo]] struct with the return value. 


#### Types

##### PtDspyQueryType 

    typedef enum
    {
        PkSizeQuery,
        PkOverwriteQuery
    }
PtDspyQueryType;

These are the query types that may be encountered by the driver in
[DspyImageQuery](#dspyimagequery).

Note: `PkSupportsCheckpointing` and `PkRenderingStartQuery` are currently not
supported by Aqsis.


##### PtDspySizeInfo

    typedef struct
    {
        PtDspyUnsigned32 width;
        PtDspyUnsigned32 height;
        PtDspyFloat32 aspectRatio;
    }
    PtDspySizeInfo;

This struct has to be returned in a query of type `PkSizeQuery`.

  * `width`: The image width in pixel
  * `height`: The image height in pixel
  * `aspectRatio`: The pixel aspect ratio


##### PtDspyOverwriteInfo

    typedef struct
    {
        PtDspyUnsigned8 overwrite;
        PtDspyUnsigned8 interactive;
    }
    PtDspyOverwriteInfo;

This struct has to be returned in a query of type `PkOverwriteQuery`.

  * `overwrite`: A boolean indicating whether the driver will overwrite an image
    or not
  * `interactive`: Unused (set this to 0).


-----


### DspyImageData 

    PtDspyError DspyImageData(PtDspyImageHandle    image,
                              int                  xmin,
                              int                  xmaxplus1,
                              int                  ymin,
                              int                  ymaxplus1,
                              int                  entrysize,
                              const unsigned char* data);

`DspyImageData` passes image pixel data from the renderer to the display device.


#### Parameters

  * `image` - handle to display internal data structures.
  * `xmin`,`ymin` - minimum x- and y-coordinates for the data.  This may be
    thought of as the top-left of the bucket.
  * `xmaxplus1`,`ymaxplus1` - maximum x- and y-coordinates for the data, plus
    one.  This means that the chunk width is `xmaxplus1-xmin` and the height is
    `ymaxplus1-ymin`.
  * `entrysize` - the stride between pixel entries in the data array.  The
    pointer `(data+entrysize)` points to the start of the data for the second
    pixel.
  * `data` - a pointer to the raw data.  The pixel data is stored with scanlines
    contiguous in memory, such that `data` points to the top-left of the chunk.


-----


### DspyImageClose

    PtDspyError DspyImageClose(PtDspyImageHandle image);

`DspyImageClose` closes the display and frees any associated resources such as
memory or file handles.


-----


### DspyImageDelayClose 

    PtDspyError DspyImageDelayClose(PtDspyImageHandle image);

This function is similar to [[#DspyImageClose|DspyImageClose]] but it is
executed in a separate process. This is useful for interactive display drivers
that want to remain active even though the renderer has finished rendering the
image.



### Additional API Types 

The following additional types are defined by the display API.

#### PtDspyError 

    typedef enum
    {
        PkDspyErrorNone,
        PkDspyErrorNoMemory,
        PkDspyErrorUnsupported,
        PkDspyErrorBadParams,
        PkDspyErrorNoResource,
        PkDspyErrorUndefined
    }
    PtDspyError;

All functions from the display driver interface are required to return a value
of type `PtDspyError` that indicates whether the function ran successfully or
not. The meaning of the values is as follows:

  * `PkDspyErrorNone`: The function ran without errors
  * `PkDspyErrorNoMemory`: The function failed to allocate memory
  * `PkDspyUnsupported`: An unsupported operation was requested
  * `PkDspyBadParams`: The driver received invalid parameters
  * `PkDspyErrorNoResource`: A required resource was not available (such as a
    file, etc.)
  * `PkDspyErrorUndefined`: No other error code was applicable


#### PtDspyImageHandle

    typedef void* PtDspyImageHandle;

The type of an image handle is known only to the display driver which uses it to
store state information between display interface function calls.  The renderer
treats this as an opaque pointer.
