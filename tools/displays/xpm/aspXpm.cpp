#include "aspXpm.h"

#include <string.h>
#include <cstdio>

aspXpm::aspXpm( const char *filename, int width, int height, int bpp )
{
  bufferSize = 256;      // default size of color buffer(70 colors);
  numColors = 0;

  aspFilename = filename;     // store the filename to open

  aspWidth = width;     // Image general info
  aspHeight = height;
  aspBpp = bpp;

  pixels.resize( width*height );    // initialize the pixels info

  memset( curtag.value, 65, 4 );      // Initialize color tags buffer
  tags.resize(bufferSize);

  cores.resize(bufferSize);         // initialize color buffer

}




aspXpm::~aspXpm()
{
}




int aspXpm::getColors()
{
  return numColors;       // returns the number of colors found in this img
}




int aspXpm::addColor( struct aspRGB cor )
{

  if( numColors >= bufferSize ) {

    bufferSize += 256;          // resize the buffers if we need more colors
    cores.resize(bufferSize);   // resize the color buffer

    tags.resize(bufferSize);    // resize the tags buffer

  }

  tags[numColors] = curtag;      // Adds the new tag to the tags buffer

  // This is the tagging system
  curtag.value[0]++;
  if( curtag.value[0] > 126 ) {
    curtag.value[0] = 65;
    curtag.value[1]++;
  }
  if( curtag.value[1] > 126 ) {
    curtag.value[0] = 65;
    curtag.value[1] = 65;
    curtag.value[2]++;
  }
  if( curtag.value[2] > 126 ) {
    curtag.value[0] = 65;
    curtag.value[1] = 65;
    curtag.value[2] = 65;
    curtag.value[3]++;
  }

  cores[numColors] = cor;       // adds the new color to the pallette
  numColors++;                  // increase the total number of colors so far

  return 1;
}




int aspXpm::saveFile()
{
  FILE *fhandle;
  unsigned int i;

  std::string fileBuffer("/* XPM */\nstatic char *");      // header of the file

  // The next 6 lines only strip the filename to provide the array name to the fileBuffer
  int pos1 = aspFilename.find_last_of('/',aspFilename.size());
  int pos2 = aspFilename.find_last_of('.',aspFilename.size());
  pos1++;
  pos1 = ( pos1==-1 ) ? 0 : pos1;
  pos2 = ( pos2==-1 )? aspFilename.length() : pos2;
  pos2 -= pos1;
  
  // adds the newly created array name buffer to the fileBuffer
  fileBuffer += aspFilename.substr(pos1,pos2);
  fileBuffer += "[] = {\n";

  fileBuffer += "/* columns rows colors chars-per-pixel */\n";        // prints the img definitions to the fileBuffer
  fileBuffer += "\"";

  std::ostringstream tempStream;
  int numCharsPerPixel;                 // Prints the number of chars needed to define 1 pixel according to the number of colors
  if( this->numColors > 226981 ) { 
    numCharsPerPixel = 4;
  } else if ( this->numColors > 3721 ) {
    numCharsPerPixel = 3;
  } else {
    numCharsPerPixel = 2;
  }
  
  tempStream << aspWidth << " " << aspHeight << " " << numColors << " " << numCharsPerPixel << " 0 0 XPMEXT\",\n";
  fileBuffer += tempStream.str();
  
  fileBuffer += "/* colors */\n";

  // iterate through the whole color buffer and formats every color entry to the fileBuffer
  for(i=0; i<numColors; i++)
  {
    tempStream.str("");     // clear string stream object

    tempStream << std::dec;
    if( numCharsPerPixel==4 ) {      // write the color tags with the required number of chars per pixel
      tempStream << "\"" << tags[i].value[0] << tags[i].value[1] << tags[i].value[2] << tags[i].value[3] << " c #";
    } else if( numCharsPerPixel==3 ) {
      tempStream << "\"" << tags[i].value[0] << tags[i].value[1] << tags[i].value[2] << " c #";
    } else {
      tempStream << "\"" << tags[i].value[0] << tags[i].value[1] << " c #";
    }

    fileBuffer += tempStream.str();     // write to fileBuffer

    tempStream.str("");     // clear string stream object

    // for every color component write out as hex
    tempStream << std::setfill('0') << std::setw(2);
    tempStream << std::hex << (unsigned int) cores[i].r;
    tempStream << std::setfill('0') << std::setw(2);
    tempStream << std::hex << (unsigned int) cores[i].g;
    tempStream << std::setfill('0') << std::setw(2);    
    tempStream << std::hex << (unsigned int) cores[i].b << "\",\n";
    
    fileBuffer += tempStream.str();
  }

  fileBuffer += "/* pixels */\n";     // start the pixels section

  // iterate through the pixels color-index entry buffer and adds the respective color tag to the fileBuffer
  for(i=0; i< (unsigned int) aspHeight; i++)
  {
    tempStream.str("");     // cleae string stream object

    tempStream << "\"";
    for(int j=0; j<aspWidth; j++)
    {
      int current = ( i * aspWidth + j );
      if( numCharsPerPixel==4 ) {     // write the real pixel information according to the number of chars per pixel
        tempStream << tags[pixels[current]].value[0] << tags[pixels[current]].value[1] << tags[pixels[current]].value[2] << tags[pixels[current]].value[3];
      } else if( numCharsPerPixel==3 ) {
        tempStream << tags[pixels[current]].value[0] << tags[pixels[current]].value[1] << tags[pixels[current]].value[2];
      } else {
        tempStream << tags[pixels[current]].value[0] << tags[pixels[current]].value[1];
      }
    }
    tempStream << "\"," << std::endl;
    fileBuffer += tempStream.str();
  }

  fileBuffer += "\"XPMEXT none none\",\n";
  fileBuffer += "\"XPMENDEXT\"\n};";

  fhandle = fopen( aspFilename.c_str(), "wb" );      // open the desired file

  if( !fhandle ) {      // check for errors opening file
    std::cerr << "XPM_ERROR: Unable to open/create file " << aspFilename << std::endl;
    return -1;
  }

  size_t len_written = fwrite( fileBuffer.c_str(), sizeof( char ), fileBuffer.length() , fhandle );      // write contents of fileBuffer to the file
  if(len_written != fileBuffer.length())
	return -1;

  fclose( fhandle );      // closes the file

  return 1;
}




int aspXpm::colorExists( struct aspRGB cor )
{
  for(unsigned int i=0; i<numColors; i++)     // walks the whole array to check if the color already exists
  {
    if( cores[i].r == cor.r && cores[i].g == cor.g && cores[i].b == cor.b ) {
      return i;
    }
  }
  
  return -1;
}




int aspXpm::bpp()
{
  return this->aspBpp;
}




int aspXpm::width()
{
  return this->aspWidth;
}




int aspXpm::height()
{
  return this->aspHeight;
}




int aspXpm::processData( void *th, int x, int y, int xmax, int ymax, const unsigned char *data )
{
  int curPixel=0;
  struct aspRGBA *rgbaImg = (struct aspRGBA *) data;
  struct aspRGB * rgbImg = (struct aspRGB *) data;
  struct aspRGB tempColor;      // var for storing color to test if exists
  int retVal;     // general variable
  aspXpm * t = (aspXpm *) th;

  int curX = x;
  int curY = y;

  while( curY < ymax ){
    curX = x;
    while( curX < xmax ) {
      if( t->aspBpp==3){
        tempColor.r = rgbImg[curPixel].r;
        tempColor.g = rgbImg[curPixel].g;
        tempColor.b = rgbImg[curPixel].b;
      } else {
        tempColor.r = rgbaImg[curPixel].r;
        tempColor.g = rgbaImg[curPixel].g;
        tempColor.b = rgbaImg[curPixel].b;
      }
      retVal = colorExists( tempColor );              // checks if the current color already exists in color pallete
      if( retVal!=-1 ) {
        pixels[curY*aspWidth+curX]=retVal;            // add color palette index into pixels buffer
      } else {
        addColor( tempColor );                        // if the color doesn't exist in the palette yet then simply add it
        pixels[curY*aspWidth+curX]=numColors-1;       // if the color didn't exist we know it's location is the number of colors-1 (it starts in 0)
      }
      curPixel++;     // increase globa pixel
      curX++;         // increase current X rendering pixel
    }
    curY++;           // increase current rendering line
  }
    
  return 1;
}


