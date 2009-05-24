#ifndef aspXpm_H
#define aspXpm_H

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <iosfwd>
#include <vector>

struct aspRGB
{
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

struct aspRGBA
{
  unsigned char a;
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

struct tag
{
  unsigned char value[4];
};

class aspXpm
{
  public:

  aspXpm(const char *filename, int width, int height, int bpp);
  ~aspXpm();
  int processData( void *th, int x, int y, int xmax, int ymax, const unsigned char *data );     // processes one bucket of rendered pixels

  int getColors();      // return the number of colors in the image
  int saveFile();       // saves the desired .xpm image
  int bpp();         // returns the number of bytes per pixel in this image
  int width();       // returns image width
  int height();      // returns image height

  private:

  int addColor( struct aspRGB cor );      // adds a new color to the buffer and resizes the buffers if it needs to
  int colorExists( struct aspRGB cor );   // iterates throught the whole buffer checking if the provided color is already there. returns the index

  std::string aspFilename;                // this will contain the string with the filename
  std::vector<struct tag> tags;           // buffer that will contain the color tags, 3 chars per color (max 65536 colors)
  std::vector<struct aspRGB> cores;       // color buffer
  std::vector<unsigned int> pixels;       // buffer that will contain the indexes into the palette buffer
  unsigned long bufferSize;               // the current size of the color buffer
  unsigned long numColors;                // the number of colors in the image
  struct tag curtag;                      // variable to keep track of the tags chars
  int aspBpp;                             // image number of bytes per pixel
  int aspWidth;                           // image width
  int aspHeight;                          // image height

};

#endif


