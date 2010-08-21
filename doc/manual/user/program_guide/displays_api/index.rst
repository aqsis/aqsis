.. include:: ../../common.rst

.. _displays_api:

======================
The Display Driver API
======================

Overview
--------

A display driver is a shared library that gets loaded by the renderer at runtime and receives the final rendered image in individual chunks (buckets or scanlines). The display driver is responsible for either displaying the image to the user, storing it on disk, or whatever else is required in the rendering pipeline.

Displays are connected to the renderer via the **RiDisplay()** interface call or the **Display** RIB request (see the RISpec for details).  Although the method for connecting displays is well-defined by the standard, the display interface itself isn't formally described.  Nevertheless, the "Dspy" interface used by Pixar's PRMan provides a de-facto standard which is implemented in |Aqsis| and most other RenderMan renderers.  Implementing the same interface means that compiled display drivers should be compatible between the various RenderMan renderers.

|Aqsis| includes some standard display drivers that allow you to save the image in various file formats or display it directly in a framebuffer.  However, it is also possible to write your own customized display drivers using the interface described in this section.


Display Functions
^^^^^^^^^^^^^^^^^

Each display driver following the Dspy standard must implement four mandatory functions:

* DspyImageOpen_ is the function which sets up the display.  It is called before the rendering starts.
* DspyImageQuery_ allows |Aqsis| to ask the display for additional information about an image.
* DspyImageData_ is called to pass the image data from the renderer to the display.
* DspyImageClose_ is called when the entire image has been rendered so that the display driver may free its resources.

There is one optional function:

* DspyImageDelayClose_ may be used by interactive display drivers so that they can remain open while the renderer shuts down.

All functions are explained in detail in the `API Reference`_.


Example
-------

The following is an example display driver that just dumps the functions with their arguments to stdout 

.. code-block:: cpp

    /*
      Demo Display Driver that just dumps all the data to stdout.
    */

    #include <ndspy.h>
    #include <string.h>
    #include <iostream>

    /* ------------------------------------------------------------
      DspyImageOpen
     ------------------------------------------------------------ */
    PtDspyError DspyImageOpen(PtDspyImageHandle * image,
                  const char *drivername,
                  const char *filename,
                  int width,
                  int height,
                  int paramCount,
                  const UserParameter *parameters,
                  int formatCount,
                  PtDspyDevFormat *format,
                  PtFlagStuff *flagStuff)
    {
      std::cout<<"ImageOpen:"<<std::endl;
      std::cout<<"  Driver name    : '"<<drivername<<"'"<<std::endl;
      std::cout<<"  File name      : '"<<filename<<"'"<<std::endl;
      std::cout<<"  Width x Height : "<<width<<"x"<<height<<std::endl;
      std::cout<<"  #Parameters    : "<<paramCount<<std::endl;
      std::cout<<"  #Channels      : "<<formatCount<<std::endl;

      // Dump the parameters
      for(int i=0; i<paramCount; i++)
      {
        const UserParameter& param = parameters[i];
        std::cout<<"  Parameter "<<i+1<<": "<<param.name<<" ("<<param.vtype<<", "<<(int)param.vcount<<") = ";
        for (int j=0; j<param.vcount; j++)
        {
          switch(param.vtype)
          {
            case 'i': std::cout<<*((int*)param.value+j)<<" "; break;
            case 'f': std::cout<<*((float*)param.value+j)<<" "; break;
            case 's': std::cout<<*((char**)param.value+j)<<" "; break;
            default: std::cout<<"unknown type"; 
          }
        }
        std::cout<<std::endl;
      }

      // Dump the channels
      for(int i=0; i<formatCount; i++)
      {
        PtDspyDevFormat& f = format[i];
        std::cout<<"  Channel '"<<f.name<<"': ";
        switch(f.type)
        {
        case PkDspyNone: std::cout<<"None"; break;
        case PkDspyFloat32: std::cout<<"Float32"; break;
        case PkDspyUnsigned32: std::cout<<"Unsigned32"; break;
        case PkDspySigned32: std::cout<<"Signed32"; break;
        case PkDspyUnsigned16: std::cout<<"Unsigned16"; break;
        case PkDspySigned16: std::cout<<"Signed16"; break;
        case PkDspyUnsigned8: std::cout<<"Unsigned8"; break;
        case PkDspySigned8: std::cout<<"Signed8"; break;
        default: std::cout<<"Unknown";
        }
        std::cout<<std::endl;
      }

      flagStuff->flags = 0;
      return PkDspyErrorNone;
    }

    /* ------------------------------------------------------------
      DspyImageQuery
     ------------------------------------------------------------ */
    PtDspyError DspyImageQuery(PtDspyImageHandle image,
                   PtDspyQueryType type,
                   int size,
                   void *data)
    {
      std::cout<<"ImageQuery()"<<std::endl;
      std::cout<<"  Type: ";
      switch(type)
      {
        case PkSizeQuery:
        {
          std::cout<<"SizeQuery";
          PtDspySizeInfo info;
          info.width = 320;
          info.height = 160;
          info.aspectRatio = 1.0f;
          if (size>sizeof(info))
            size = sizeof(info);
          memcpy(data, &info, size);
          break;
        }
        case PkOverwriteQuery:
        {
          std::cout<<"OverwriteQuery";
          PtDspyOverwriteInfo info;
          info.overwrite = 1;
          info.interactive = 0;
          if (size>sizeof(info))
            size = sizeof(info);
          memcpy(data, &info, size);
          break;
        }
        default:
          std::cout<<"Unknown query";
      }
      std::cout<<std::endl;
      std::cout<<"  Size: "<<size<<std::endl;

      return PkDspyErrorNone;
    }

    /* ------------------------------------------------------------
      DspyImageData
     ------------------------------------------------------------ */
    PtDspyError DspyImageData(PtDspyImageHandle image,
                  int xmin,
                  int xmaxplus1,
                  int ymin,
                  int ymaxplus1,
                  int entrysize,
                  const unsigned char *data)
    {
      static int count = 0;
      count++;
      int size = (xmaxplus1-xmin)*(ymaxplus1-ymin)*entrysize;
      std::cout<<count<<" - ImageData(image, "<<xmin<<", "<<xmaxplus1<<", "<<ymin<<", "<<ymaxplus1<<", "<<entrysize<<")";
      std::cout<<" - "<<size<<" bytes of pixel data"<<std::endl;

      return PkDspyErrorNone;
    }

    /* ------------------------------------------------------------
      DspyImageClose
     ------------------------------------------------------------ */
    PtDspyError DspyImageClose(PtDspyImageHandle image)
    {
      std::cout<<"ImageClose()"<<std::endl;

      return PkDspyErrorNone;
    }

Compiling and using a Display Driver
------------------------------------

A display driver must be compiled as a shared object/dynamic link library. Please consult the documentation of your compiler on more information about how this is done. Here are some example command lines: 

Linux
    With g++ a display driver may be compiled into a shared object with the command <code>g++ -fPIC -I $install_prefix/include/aqsis -shared displaydriver.cpp -o displaydriver.so</code>  The ''-fPIC'' option is probably only needed on 64-bit systems.
OSX
    The command is similar to the Linux command: <code>g++ -I /usr/local/include/aqsis -dynamiclib displaydriver.cpp -o displaydriver.dylib</code>(of course, the actual include path depends on where you have installed the |Aqsis| header files)
Windows
    Using Visual C++, we have: <code>cl /I%AQSISHOME%\include\aqsis /LD /DWIN32 /EHsc displaydriver.cpp</code> ''/EHsc'' is not required if your display driver is implemented in C.

Once the driver is compiled, you have to tell |Aqsis| that you want to use your new driver and where it will find the driver. How you do this depends on the modeling/animation package you are using - please consult the documentation of your 3D application.

If you are using |Aqsis| directly via the RenderMan C API or by creating a RIB file by hand, you can specify the driver in the **RiDisplay()** call::

    RiDisplay("out.tif", "mydriver", RI_RGB, ...paramlist...);

or as the RIB request::

    Display "out.tif" "mydriver" "rgb" ...paramlist...

|Aqsis| will then look up the name "mydriver" in an internal list that maps display names to display driver libraries. If it cannot find the name, it will output an error. You can add or modify an entry in that list using the **RiOption()** call. Here is a RIB request that associates the display name "mydriver" with the DLL "mydisplaydriver.dll"::

    Option "display" "string mydriver" ["mydisplaydriver.dll"]

The next problem is that |Aqsis| has to find the library. You could actually provide an entire path in the above call, but the better way is to add the location where you installed the driver to the list of search paths for display drivers::

    Option "searchpath" "string display" ["&:C:\RenderMan\MyDrivers"]

The **&** character represents the previously set search paths, so the above option call *adds* another path to the list. If you omit this character the path would replace the previous list and |Aqsis| would not be able to find the standard display drivers anymore (file, framebuffer, etc.). 


API Reference
-------------

|Aqsis| expects a display to implement the standard display interface as in PRMan and other renderers.  This means implementing the four functions DspyImageOpen_, DspyImageQuery_, DspyImageData_ and DspyImageClose_\. The function prototypes are defined in the header **<ndspy.h>**.

All functions in the display interface return one of the possible [[#PtDspyError|error codes]], which will normally be *PkDspyErrorNone* when everything is ok.  The first parameter to the display interface functions is an image handle of type PtDspyImageHandle_.  The handle should be used by the display driver to store internal data associated with the display.  Any handle stored here by DspyImageOpen_ will be passed on to the other functions without being modified by the renderer.

DspyImageOpen
^^^^^^^^^^^^^
.. c:function:: PtDspyError DspyImageOpen(PtDspayImageHandle* image, const char* drivername, const char* filename, int width, int height, int paramCount, const UserParameter* parameters, int formatCount, PtDspyDevFormat* format, PtFlagStuff* flagstuff)

    DspyImageOpen is called by the renderer to open and initialize a display.  Typically this might involve allocating a memory buffer large enough to hold the image, opening a file or initializing a GUI to display the data directly.

    :param image: a pointer to a handle for any internal data structures allocated by the display.  This handle should be set by ''DspyImageOpen'' and will be stored by the renderer without modification to be passed on to the other display functions.
    :type image: PtDspyImageHandle* 
    :param drivername: the name of the display driver as it was specified in the **RiDisplay()** call.
    :type drivername: const char* 
    :param filename: the output file name as given in the **RiDisplay()** call.
    :type filename: const char* 
    :param width: the image width in pixels. When the image is cropped this is the cropped size. If 0 is passed the driver must choose an appropriate default value.
    :type width: int 
    :param height: the image height in pixels. When the image is cropped this is the cropped size. If 0 is passed the driver must choose an appropriate default value.
    :type height: int 
    :param paramCount: the number of parameters stored in the array *parameters*
    :type paramCount: int 
    :param parameters: parameters to the **RiDisplay()** interface call are passed on to the display inside this array.  For each parameter there is a UserParameter_ struct that contains the name/value pair.  These structs are only valid within this function. If you need a value in one of the other functions you must copy the value! 
    :type parameters: const UserParameter* 
    :param formatCount: the number of items in the array *format* that is, the number of channels of pixel data in the image.
    :type formatCount: int 
    :param format: for each output channel, there is a PtDspyDevFormat_ struct describing the type of the data that gets passed into the driver. The driver may rearrange this array or change the data type to get the channels delivered in a different order or as a different type or in a different byte order. If you want to remove a channel, set its type to *PkDspyNone'*  When you reorder the array you must ensure that the names still point to the original strings that were passed in! Just like the *parameters* array the format structs are only valid inside this call. Do not try to access the values again in one of the other functions.
    :type format: PtDspyDevFormat* 
    :param flagstuff: the display may change the value of these flags to modify aspects of the incoming pixel data, such as to request that the renderer send scanline order.  See PtFlagStuff_ for possible values.
    :type flagstuff: PtFlagStuff* 
    :returns: An error code or PkDspyErrorNone


In addition to any user parameters passed to the driver, the array ''parameters'' will always contain the following system-defined parameters:

+--------------------+---------------+-----------------------------------------------------------------------+
| name               | vtype[vcount] | Description                                                           |
+====================+===============+=======================================================================+
| "NP"               | f[16]         | World to screen space matrix                                          |
+--------------------+---------------+-----------------------------------------------------------------------+
| "Nl"               | f[16]         | World to camera space matrix                                          |
+--------------------+---------------+-----------------------------------------------------------------------+
| "near"             | f[1]          | Near clipping plane                                                   |
+--------------------+---------------+-----------------------------------------------------------------------+
| "far"              | f[1]          | Far clipping plane                                                    |
+--------------------+---------------+-----------------------------------------------------------------------+
| "origin"           | i[2]          | Origin of the crop window within the entire image                     |
+--------------------+---------------+-----------------------------------------------------------------------+
| "OriginalSize"     | i[2]          | Original pixel size of the entire image (not just the cropped window) |
+--------------------+---------------+-----------------------------------------------------------------------+
| "PixelAspectRatio" | f[1]          | The pixel aspect ration                                               |
+--------------------+---------------+-----------------------------------------------------------------------+
| "Software"         | s[1]          | The name of the renderer                                              |
+--------------------+---------------+-----------------------------------------------------------------------+
| "HostComputer"     | s[1]          | The network name of the computer that the image is rendered on        |
+--------------------+---------------+-----------------------------------------------------------------------+

Types
"""""
.. _UserParameter:

UserParameter

.. code-block:: cpp

    typedef struct
    {
        RtToken   name;
        char      vtype;
        char      vcount;
        RtPointer value;
        int       nbytes;
    }
    UserParameter;

name
    the name of the parameter

vtype
    the type of the parameter value. This can be either ''"i"'' for integers of type ''RtInt'', ''"f"'' for floats of type ''RtFloat'' or ''"s"'' for strings of type ''char*''.

vcount
    this is the array size. If the value is a scalar, ''vcount'' is 1.

value
    this is a pointer that points to the value(s).  The display should cast this pointer to the appropriate type to access the contained data.

nbytes
    the total number of bytes used by the entire value


.. _PtDspyDevFormat:

PtDspyDevFormat

.. code-block:: cpp

    typedef struct
    {
        char *name;
        unsigned type;
    }
    PtDspyDevFormat;

name
    the name of the output channel.  Standard channels include "r", "g", "b", "a", "z", but *name* may take other values when using AOV.

type
    one of the possible `Pixel types`_ describing the channel bitwidth and type. The type can be combined (using the bitwise OR operator) with the following flags to determine the byte order of a single value.  Obviously this is only meaningful for types that are longer than a single byte.

    * PkDspyByteOrderHiLo 
      Force big endian format (most significant bytes first)
    * PkDspyByteOrderLoHi 
      Force little endian format (least significant bytes first)
    * PkDspyByteOrderNative 
      Use whatever format is native on the current machine (this is the default)


Pixel types
"""""""""""

A set of integer constants is defined to specify the output pixel format type.  All such constants start with the prefix *Pk*  Associated with these is a set of typedefs starting with the prefix *Pt*.  Pixel formats are defined both for floating point and signed/unsigned integers of different bit-widths. The value *PkDspyNone* may be set by the display to remove a channel from the data sent to the driver.  For example, if the display can only handle rgb images, but the renderer is going to create rgba data, the display should set the type for the "a" field to *PtDspyNone*.

+------------------+------------------+
| Format constant  | typedef          |
+==================+==================+
| PkDspyNone       | *nothing*        | 
+------------------+------------------+
| PkDspyFloat32    | PtDspyFloat32    | 
+------------------+------------------+
| PkDspyUnsigned32 | PtDspyUnsigned32 | 
+------------------+------------------+
| PkDspySigned32   | PtDspySigned32   | 
+------------------+------------------+
| PkDspyUnsigned16 | PtDspyUnsigned16 | 
+------------------+------------------+
| PkDspySigned16   | PtDspySigned16   | 
+------------------+------------------+
| PkDspyUnsigned8  | PtDspyUnsigned8  | 
+------------------+------------------+
| PkDspySigned8    | PtDspySigned8    | 
+------------------+------------------+


.. _PtFlagStuff:

PtFlagStuff

.. code-block:: cpp

    typedef struct
    {
        int flags;
    }
    PtFlagStuff;

flags
    is a bitwise or of the following constants:

    * PkDspyFlagsWantsScanLineOrder
      Send the data in scanline order rather than as buckets.  Scanline order starts from the top-left of the image, working downward line by line.  Note that the display shouldn't assume that a whole scanline will arrive at once.
    * PkDspyFlagsWantsEmptyBuckets
      Send pixel data even for buckets which contain no primitives.
    * PkDspyFlagsWantsNullEmptyBuckets
      Generate calls to ''DspyImageData'' for empty buckets, but set the associated *data* pointer to null.


DspyImageQuery
^^^^^^^^^^^^^^
.. c:function:: PtDspyError DspyImageQuery(PtDspyImageHandle image, PtDspyQueryType type, size_t size, void* data)

    This function is used by the renderer to obtain further information about the image or the display driver itself. |Aqsis| passes in the image handle that was initialised in the open function (or *NULL* if the query does not relate to a particular image), a query type that determines what kind of information is requested and a data block that has to be filled in by the driver with appropriate values.

    :param image: this is either the image handle that was returned in DspyImageOpen_ or a *NULL* pointer.
    :type image: PtDspyImageHandle 
    :param type: determines the type of information that is requested. The possible types are defined in the enum PtDspyQueryType_.
    :type type: PtDspyQueryType 
    :param size: the size in bytes of the memory that *data* points to. Make sure not to write beyond this size!
    :type size: size_t 
    :param data: an allocated memory buffer of *size* bytes that has to be filled in by the display driver. Depending on the query type this points to an appropriate info struct (see below).
    :type data: void* 
    :returns: An error code or PkDspyErrorNone


The following requests may be made by the renderer:

.. _PkSizeQuery:

PkSizeQuery
    The renderer wants to know the pixel size of an image and the pixel aspect ratio (when *image* is *NULL*, default values have to be returned). The values have to be filled into a PtDspySizeInfo_ structure and copied to *data*.

.. _PkOverwriteQuery:

PkOverwriteQuery
    This is used to query the driver whether it will overwrite an image file or not. For this query, *data* should receive a PtDspyOverwriteInfo_ struct with the return value. 


Types
"""""

.. _PtDspyQueryType:

PtDspyQueryType

.. code-block:: cpp

    typedef enum
    {
        PkSizeQuery,
        PkOverwriteQuery
    }
    PtDspyQueryType;

These are the query types that may be encountered by the driver in DspyImageQuery_.

.. note:: PkSupportsCheckpointing and PkRenderingStartQuery are currently not supported by |Aqsis|.

.. _PtDspySizeInfo:

PtDspySizeInfo

.. code-block:: cpp

    typedef struct
    {
        PtDspyUnsigned32 width;
        PtDspyUnsigned32 height;
        PtDspyFloat32 aspectRatio;
    }
    PtDspySizeInfo;

This struct has to be returned in a query of type PkSizeQuery_.

width
    The image width in pixel

height
    The image height in pixel

aspectRatio
    The pixel aspect ratio


.. _PtDspyOverwriteInfo:

PtDspyOverwriteInfo

.. code-block:: cpp

    typedef struct
    {
        PtDspyUnsigned8 overwrite;
        PtDspyUnsigned8 interactive;
    }
    PtDspyOverwriteInfo;

This struct has to be returned in a query of type ''PkOverwriteQuery''.

overwrite
    A boolean indicating whether the driver will overwrite an image or not

interactive
    Unused (set this to 0).



DspyImageData
^^^^^^^^^^^^^
.. c:function:: PtDspyError DspyImageData(PtDspyImageHandle image, int xmin, int xmaxplus1, int ymin, int ymaxplus1, int entrysize, const unsigned char* data)

    DspyImageData passes image pixel data from the renderer to the display device.

    :param image: handle to display internal data structures.
    :type image: PtDspyImageHandle
    :param xmin: minimum x-coordinate for the data.  This may be thought of as the left of the bucket.
    :type xmin: int
    :param ymin: minimum y-coordinate for the data.  This may be thought of as the top of the bucket.
    :type ymin: int
    :param xmaxplus1: maximum x-coordinate for the data, plus one.  This means that the chunk width is *xmaxplus1*\ -\ *xmin*.
    :type xmaxplus1: int
    :param ymaxplus1: maximum y-coordinate for the data, plus one.  This means that the chunk height is *ymaxplus1*\ -\ *ymin*.
    :type ymaxplus1: int
    :param entrysize: the stride between pixel entries in the data array.  The pointer *(data+entrysize)* points to the start of the data for the second pixel.
    :type entrysize: int
    :param data: a pointer to the raw data.  The pixel data is stored with scanlines contiguous in memory, such that *data* points to the top-left of the chunk.
    :type data: const unsigned char*
    :returns: An error code or PkDspyErrorNone


DspyImageClose
^^^^^^^^^^^^^^
.. c:function:: PtDspyError DspyImageClose(PtDspyImageHandle image);

    DspyImageClose closes the display and frees any associated resources such as memory or file handles.

    :param image: handle to display internal data structures.
    :type image: PtDspyImageHandle
    :returns: An error code or PkDspyErrorNone


DspyImageDelayClose
^^^^^^^^^^^^^^^^^^^
.. c:function:: PtDspyError DspyImageDelayClose(PtDspyImageHandle image);

    This function is similar to DspyImageClose_ but it is executed in a separate process. This is useful for interactive display drivers that want to remain active even though the renderer has finished rendering the image.


Additional API Types
--------------------

The following additional types are defined by the display API.

.. _PtDspyError:

PtDspyError

.. code-block:: cpp

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

All functions from the display driver interface are required to return a value of type ''PtDspyError'' that indicates whether the function ran successfully or not. The meaning of the values is as follows:

  * PkDspyErrorNone
    The function ran without errors
  * PkDspyErrorNoMemory
    The function failed to allocate memory
  * PkDspyUnsupported
    An unsupported operation was requested
  * PkDspyBadParams
    The driver received invalid parameters
  * PkDspyErrorNoResource
    A required resource was not available (such as a file, etc.)
  * PkDspyErrorUndefined
    No other error code was applicable


.. _PtDspyImageHandle:

PtDspyImageHandle

.. code-block:: cpp

    typedef void* PtDspyImageHandle;

The type of an image handle is known only to the display driver which uses it to store state information between display interface function calls.  The renderer treats this as an opaque pointer.
