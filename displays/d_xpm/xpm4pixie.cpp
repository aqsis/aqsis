//
//  Author: Alex Paes
//  
//  Project: XPM driver for 3Delight/PRMan/RenderDotC and any PRMan display driver compatible renderer
//
//  Description: 
//  This driver outputs .xpm files with a minimum size of 16x16 and a max of 256x256.
//  It parses the given pixel data and will build a color palette for the file.


#include "/Applications/Graphics/Pixie/include/dsply.h"
#include <aspXpm.h>
#include <iostream>

void *displayStart( const char *filename, int width, int height, int bpp, const char *chans, TDisplayParameterFunction parameterFunc )
{
  int retVal;
  std::string channels(chans);
  
  if( !filename || strlen(filename)==0 ) {     // filename must exist to output xpm file
    std::cerr << "XPM_ERROR: No filename provided for output" << std::endl;
    return 0;
  }
  
  retVal = strlen( filename );      // will only proceed if the filename fits the predefined buffer for the name
  if( retVal > 256 ) {
    std::cerr << "XPM_ERROR: Maximum filename size is 256 characters long" << std::endl;
    return 0;
  }
  
  if( width<16 || width>3072 || height<16 || height>3072 ) {
    std::cerr << "XPM_ERROR: This driver is limited to 3072x3072 output" << std::endl;
    return 0;
  }
  
  if( channels!="rgba" && channels!="rgb" ) {     // checks if it is going to render rgb or rgba
    std::cerr << "Only RGB or RGBA channels supported" << std::endl;
    return 0;
  }
  
  aspXpm *xpmImg = new aspXpm( filename, width, height, channels.length() );     // Create new xpmImg instance where all our img info will be stored
  if( !xpmImg ) {
    std::cerr << "XPM_ERROR: Unable to allocate xpm image buffer" << std::endl;
    return 0;
  }
  
  return (void *) xpmImg;
}

                          
int displayData( void *image,
                 int x,
                 int y,
                 int w,
                 int h,
                 float *data
               )
{
  unsigned char *imgData;
  int curByte=0;
  int value;
  float valFloat;
  
  int curY = 0;
  int curX = 0;
  
  aspXpm *xpmImg = (aspXpm *) image;
  
  int fullWidth = w * xpmImg->bpp();
  
  int buff=0;
  buff = h * fullWidth;
  
  imgData = new unsigned char[buff];
  
  while( curY < h ) {
    curX = 0;
    while( curX < fullWidth ) {
      curByte = curY * fullWidth + curX;

      valFloat = data[curByte];
      valFloat = (valFloat>0.95)? 0.95 : valFloat;
      valFloat = (valFloat<0.05)? 0.05 : valFloat;
      valFloat *= 255;

      value = (unsigned int) valFloat;

      imgData[curByte] = (unsigned char) value;

      curX++;
    }
    curY++;
  }

  if( !xpmImg || !imgData ) {
    std::cerr << "ERROR: buffer missing" << std::endl;
    xpmImg->~aspXpm();
    return 0;
  }
  
  if( !xpmImg->processData( image, x, y, x+w, y+h, imgData) ) {
    std::cerr << "ERROR: processing data" << std::endl;
    xpmImg->~aspXpm();
    return 0;
  }
  
  delete[] imgData;
  
  return 1;
}
                          

void displayFinish( void *image )
{
  aspXpm *xpmImg = (aspXpm *) image;
  
  xpmImg->saveFile();
}

