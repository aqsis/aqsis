---
layout: documentation
title: Example 
category: display_driver_api
order: 10
---


Example
-------

The following is an example display driver that just dumps the functions with
their arguments to stdout. 

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



#### Compiling and using a Display Driver

A display driver must be compiled as a shared object/dynamic link library.
Please consult the documentation of your compiler on more information about how
this is done. Here are some example command lines: 

  * **Linux**: With g++ a display driver may be compiled into a shared object
    with the command 
    
    ~~~~~
    g++ -fPIC -I $install_prefix/include/aqsis -shared displaydriver.cpp -o displaydriver.so
    ~~~~~
    
    The `-fPIC` option is probably only needed on 64-bit systems.

  * **OSX**: The command is similar to the Linux command: 
  
    ~~~~~
    g++ -I /usr/local/include/aqsis -dynamiclib displaydriver.cpp -o displaydriver.dylib
    ~~~~~
    
    (of course, the actual include path depends on where you have installed the
    Aqsis header files)

  * **Windows** Using Visual C++, we have: 
  
    ~~~~~
    cl /I%AQSISHOME%\include\aqsis /LD /DWIN32 /EHsc displaydriver.cpp
    ~~~~~
    
    `/EHsc` is not required if your display driver is implemented in C.

Once the driver is compiled, you have to tell Aqsis 1) that you want to use your
new driver and 2) where it will find the driver. How you do this depends on the
modeling/animation package you are using - please consult the documentation of
your 3D application.

If you are using Aqsis directly via the RenderMan C API or by creating a RIB
file by hand, you can specify the driver in the `RiDisplay()` call:

    RiDisplay("out.tif", "mydriver", RI_RGB, ...paramlist...);

or as the RIB request:

    Display "out.tif" "mydriver" "rgb" ...paramlist...

Aqsis will then look up the name "mydriver" in an internal list that maps
display names to display driver libraries. If it cannot find the name, it will
output an error. You can add or modify an entry in that list using the
`RiOption()` call. Here is a RIB request that associates the display name
"mydriver" with the DLL "mydisplaydriver.dll":

    Option "display" "string mydriver" ["mydisplaydriver.dll"]

The next problem is that Aqsis has to find the library. You could actually
provide an entire path in the above call, but the better way is to add the
location where you installed the driver to the list of search paths for display
drivers:

    Option "searchpath" "string display" ["&:C:\RenderMan\MyDrivers"]

The `&` character represents the previously set search paths, so the above
option call //adds// another path to the list. If you omit this character the
path would replace the previous list and Aqsis would not be able to find the
standard display drivers anymore (`file`, `framebuffer`, etc.). 



