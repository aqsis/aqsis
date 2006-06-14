// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** \file
		\brief Implements a EXR importer
		\author Michel Joron (joron@sympatico.ca)
*/


// The conversion process assumes every single .exr will have RGBA channels
// and will produce a Float tiff file equivalent to to the .exr provided.
// Of course; the size of the resulted tiff file will be huge.
//
// The conversion will create a valid .tif file and return
// the filename of the new file created.
// Eg. this function exr2tif() created a .tif file at the same place
// as the .exr filename provided.
//
//
// IN ORDER TO CONVERSION OCCURRED the dll/dso must be present in
// procedures directory under <AQSIS_BASE_PATH>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern "C"
{
#include "../common/pixelsave.h"
}
/* #define MAIN / usefull to debug an autonomous exr2tif converter */

#include <ImfRgbaFile.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfArray.h>

#include <iostream>

using namespace std;
using namespace Imf;
using namespace Imath;

// Replacement for Ilm::IsIlmMagic()
static bool IsMagic(char bytes[])
{
	bool retval = false;
	if ((bytes[0] == 0x76) &&
	        (bytes[1] == 0x2f) &&
	        (bytes[2] == 0x31) &&
	        (bytes[3] == 0x01) )
	{
		retval = true;
	}
	return retval;
}

// Test if the file is indeed a OpenEXR texture file
static bool isOpenExrFile (const char fileName[])
{
	char bytes[4];
	bool retval = false;
	FILE *fp = NULL;

	if (fileName && *fileName)
		fp = fopen(fileName,"rb");

	if (fp)
	{
		int rd = fread(bytes, 1, sizeof(bytes), fp);
		fclose(fp);
		retval = (rd == 4) && IsMagic(bytes); // IsIlmMagic(bytes) is not found by VC7.1 ???
	}


	return retval;
}

static float
knee (float x, float f)
{
	return log (x * f + 1) / f;
}


static unsigned char
gamma (half h, float m)
{
	//
	// Conversion from half to unsigned char pixel data,
	// with gamma correction.  The conversion is the same
	// as in the exrdisplay program's ImageView class,
	// except with defog, kneeLow, and kneeHigh fixed
	// at 0.0, 0.0, and 5.0 respectively.
	//

	float x = max (0.f, h * m);

	if (x > 1)
		x = 1 + knee (x - 1, 0.184874f);

	return (unsigned char) (clamp (Math<float>::pow (x, 0.4545f) * 84.66f,
	                               0.f,
	                               255.f));
}

// Shameless copied from an example rgbaInterfaceExamples.cpp
static void readRgba1 (const char fileName[],
                       Array2D<Rgba> &pixels,
                       int &width,
                       int &height)
{
	//
	// Read an RGBA image using class RgbaInputFile:
	//
	//	- open the file
	//	- allocate memory for the pixels
	//	- describe the memory layout of the pixels
	//	- read the pixels from the file
	//

	RgbaInputFile file (fileName);
	Box2i dw = file.dataWindow();

	width  = dw.max.x - dw.min.x + 1;
	height = dw.max.y - dw.min.y + 1;
	pixels.resizeErase (height, width);

	file.setFrameBuffer (&pixels[0][0] - dw.min.x - dw.min.y * width, 1, width);
	file.readPixels (dw.min.y, dw.max.y);
}



static char tiffname[1024];

#ifdef	WIN32
#define	__export	__declspec(dllexport)
#else	// WIN32
#define	__export
#endif	// WIN32

/* Main function to convert any exr to tif format
 */
extern "C" __export char *exr2tif(char *in)
{
	char *result =NULL;
	int width,height;
	Array2D<Rgba> rgba;
	unsigned char *pixels;

	strcpy(tiffname, in);
	if ((result = strstr(tiffname, ".exr")) != 0)
		strcpy(result, ".tif");
	if (!result)
	{
		if ((result = strstr(tiffname, ".EXR")) != 0)
			strcpy(result, ".tif");
	}

	// test if it indeed an OpenEXR file
	//    1) the extension .exr or .EXR must be present
	//    2) its magic number should match
	if ( !result || !isOpenExrFile (in) )
		return NULL;


	// Read all the pixels into Half Array 'rgba'
	readRgba1(in, rgba, width, height);


	// hopefully it contains something
	if (width == 0 || height == 0)
		return NULL;

	int n = 4 * width;

	float exposure = 0.0;


	result = ( char * ) getenv( "GAMMA" );
	float sign = 1.0f;

	if ( result && ( result[ 0 ] == '-' ) )
	{
		result++;
		sign = -1.0f;
	}

	if ( result && ( result[ 0 ] >= '0' ) && ( result[ 0 ] <= '9' ) )
	{
		sscanf(result, "%f", &exposure);
		exposure *= sign;
	}

	// width * height * 4 floats
	pixels = (unsigned char *) malloc(n * sizeof(unsigned char) *  height);
	float m  = Math<float>::pow (2.f, clamp (exposure + 2.47393f, -20.f, 20.f));

	for (int i=0; i< height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			pixels[i*n + 4*j]     = gamma(rgba[i][j].r, m); // red
			pixels[i*n + 4*j + 1] = gamma(rgba[i][j].g, m); // green
			pixels[i*n + 4*j + 2] = gamma(rgba[i][j].b, m); // blue
			pixels[i*n + 4*j + 3] = rgba[i][j].a * 255; // alpha
		}
	}

	// a function to save as Float tiff files
	save_tiff( tiffname, pixels, width, height, 4, "exr2tif" );

	free(pixels);

	return tiffname;
}

#ifdef MAIN
int main(int argc, char *argv[])
{
	char *result;

	if (argc != 2)
	{
		fprintf(stderr, "Usage %s: %s some.exr", argv[0], argv[1]);
		exit(2);
	}
	result = exr2tif(argv[1]);
	if (result)
	{
		puts(result);
	}
	return 1;
}
#endif
