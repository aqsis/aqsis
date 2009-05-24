//
//  Author: Alex Paes
//  
//  Project: XPM driver for 3Delight/PRMan/RenderDotC and any PRMan display driver compatible renderer
//
//  Description: 
//  This driver outputs .xpm files with a minimum size of 16x16 and a max of 256x256.
//  It parses the given pixel data and will build a color palette for the file.


#include <iostream>
#include <string.h>

#undef DSPY_INTERNAL
#include <aqsis/ri/ndspy.h>
#include "aspXpm.h"

static aspXpm *xpmImg;

extern "C" PtDspyError DspyImageQuery ( PtDspyImageHandle image, PtDspyQueryType type, size_t size, void *data )
{
  if( size<=0 || !data ) {
    return PkDspyErrorBadParams;
  }
  
  //aspXpm *xpmImg = (aspXpm *) image;
  
  switch( type ) {
    case PkSizeQuery: {
      
      PtDspySizeInfo defaultSize;
      
      if( xpmImg ) {
        defaultSize.width = xpmImg->width();
        defaultSize.height = xpmImg->height();
        defaultSize.aspectRatio = 1;
      } else {
        defaultSize.width        = 128;
        defaultSize.height       = 128;
        defaultSize.aspectRatio  = 1;
      }
      
      memcpy( data, &defaultSize, (size>sizeof(defaultSize)? sizeof(defaultSize) : size ) );
      
      break;
    }
    
    case PkOverwriteQuery: {
      
      PtDspyOverwriteInfo doOverwrite;
      
      doOverwrite.overwrite = 1;
      
      memcpy( data, &doOverwrite, (size>sizeof(doOverwrite)? sizeof(doOverwrite) : size ) );
      
      break;
    }
    
    default: {
      return PkDspyErrorUnsupported;
    }
  }
  
  //std::cout << "exiting DspyImageQuery" << std::endl;
  return PkDspyErrorNone;
}

extern "C" PtDspyError DspyImageOpen(PtDspyImageHandle *image,
                                     const char *drivername,
                                     const char *filename,
                                     int width,
                                     int height,
                                     int paramcount,
                                     const UserParameter *parameters,
                                     int formatcount,
                                     PtDspyDevFormat *format,
                                     PtFlagStuff *flagsstuff )
{
  //std::cout << "Entering DspyImageOpen" << std::endl;
  
  int retVal;
  std::string channels("");
  
  if( !filename || strlen(filename)==0 ) {     // filename must exist to output xpm file
    std::cerr << "XPM_ERROR: No filename provided for output" << std::endl;
    return PkDspyErrorBadParams;
  }
  
  retVal = strlen( filename );      // will only proceed if the filename fits the predefined buffer for the name
  if( retVal > 256 ) {
    std::cerr << "XPM_ERROR: Maximum filename size is 256 characters long" << std::endl;
    return PkDspyErrorBadParams;
  }
  
  if( width<16 || width>3072 || height<16 || height>3072 ) { return PkDspyErrorUnsupported; }
  
  if( formatcount<3 || formatcount>4 ) {
    return PkDspyErrorUnsupported;
  }
  
  for( int i=0, iSize=formatcount; i<iSize; i++) {      // stores the whole array of channels in channels var
    channels+=format[i].name;
  }
 
  if( (channels!="rgba") && (channels!="rgb") && (channels !="argb")) {     // checks if it is going to render rgb or rgba
    std::cerr << "Only RGB or RGBA channels supported" << std::endl;
    return PkDspyErrorUnsupported;
  }
  
  
  xpmImg = new aspXpm( filename, width, height, channels.length() );     // Create new xpmImg instance where all our img info will be stored
  if( !xpmImg ) {
    std::cerr << "XPM_ERROR: Unable to allocate xpm image buffer" << std::endl;
    return PkDspyErrorBadParams;
  }
  
  *image = (void *) xpmImg;
  
  //flagsstuff->flags |= PkDspyFlagsWantsScanLineOrder;
  flagsstuff->flags |= PkDspyFlagsWantsEmptyBuckets;
  
  //std::cout << "Exiting DspyImageOpen" << std::endl;
  return PkDspyErrorNone;
}

                          
extern "C" PtDspyError DspyImageData ( PtDspyImageHandle image,
                            int xmin,
                            int xmax_plus_one,
                            int ymin,
                            int ymax_plus_one,
                            int entrysize,
                            const unsigned char *data
                          )
{
  //std::cout << "Entering DspyImageData" << std::endl;
 // aspXpm *xpmImg = (aspXpm *) image;
  unsigned char *imgData = (unsigned char *) data;
  
  if( !xpmImg || !imgData ) {
    return PkDspyErrorBadParams;
  }
  
  if( entrysize<3 || entrysize>4 ) {
    return PkDspyErrorBadParams;
  }
  
  if( !xpmImg->processData( xpmImg, xmin, ymin, xmax_plus_one, ymax_plus_one, imgData) ) {
    return PkDspyErrorNoResource;
  }
  
  //std::cout << "Exiting DspyImageData" << std::endl;
  return PkDspyErrorNone;
}
                          

extern "C" PtDspyError DspyImageClose ( PtDspyImageHandle image )
{
  //std::cout << "Entering DspyImageClose" << std::endl;
  
  //aspXpm *xpmImg = (aspXpm *) image;
  
  //std::cout << "Number of distinct colors so far: " << xpmImg->getColors() << std::endl;

  if( !xpmImg->saveFile() ) {
    return PkDspyErrorUndefined;
  }
  
  //delete xpmImg;
  
  //std::cout << "Exiting DspyImageClose" << std::endl;
  return PkDspyErrorNone;
}

