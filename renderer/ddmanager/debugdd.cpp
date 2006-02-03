//
//  Author: Tristan Colgate-McFarlane
//  Author: Derived from d_xpm by Alex Paes
//  
//  Project: This is a debug DD intended for internal linking into aqsis.
//
//  Description: 
//    This DD is intended for internal linking into Aqsis. It is intended as
//    a debugging aide to support a completely static build of Aqsis suitable
//    for debugging in situation where dynamic linking causes problems
//      The intent is to behave as much like a real dd as possible, without doing
//    anything useful


#include <iostream>
#undef DSPY_INTERNAL
#include "ndspy.h"
#include "debugdd.h"

PtDspyError DebugDspyImageQuery ( PtDspyImageHandle image, PtDspyQueryType type, size_t size, void *data )
{
  if( size<=0 || !data ) {
    return PkDspyErrorBadParams;
  }
  
  //aspXpm *xpmImg = (aspXpm *) image;
  
  switch( type ) {
    case PkSizeQuery: {
      
      PtDspySizeInfo defaultSize;

      defaultSize.width        = 128;
      defaultSize.height       = 128;
      defaultSize.aspectRatio  = 1;
      
      break;
    }
    
    case PkOverwriteQuery: {
      
      PtDspyOverwriteInfo doOverwrite;
      
      doOverwrite.overwrite = 1;
      
      break;
    }
    
    default: {
      return PkDspyErrorUnsupported;
    }
  }
  
  //std::cout << "exiting DspyImageQuery" << std::endl;
  return PkDspyErrorNone;
}

PtDspyError DebugDspyImageOpen ( PtDspyImageHandle *image,
                            const char *drivername,
                            const char *filename,
                            int width,
                            int height,
                            int paramcount,
                            const UserParameter *parameters,
                            int formatcount,
                            PtDspyDevFormat *format,
                            PtFlagStuff *flagsstuff
                          )
{
  // SHould do more useful things in here, output channels and so on that are
  // being rendered.
 
  //std::cout << "Entering DspyImageOpen" << std::endl;
  
  flagsstuff->flags |= PkDspyFlagsWantsEmptyBuckets;
  
  //std::cout << "Exiting DspyImageOpen" << std::endl;
  return PkDspyErrorNone;
}

                          
PtDspyError DebugDspyImageData ( PtDspyImageHandle image,
                            int xmin,
                            int xmax_plus_one,
                            int ymin,
                            int ymax_plus_one,
                            int entrysize,
                            const unsigned char *data
                          )
{
  //std::cout << "Entering DspyImageData" << std::endl;
  
  // Should do something useful here with the bucket data.
  
  //std::cout << "Exiting DspyImageData" << std::endl;
  return PkDspyErrorNone;
}
                          

PtDspyError DebugDspyImageClose ( PtDspyImageHandle image )
{
  //std::cout << "Entering DspyImageClose" << std::endl;
  
  return PkDspyErrorNone;
}

PtDspyError DebugDspyDelayImageClose ( PtDspyImageHandle image )
{
  //std::cout << "Entering DspyImageClose" << std::endl;
  
  return PkDspyErrorNone;
}



