/******************************************************************************/
/* COPYRIGHT                                                                  */
/*                                                                            */
/* Copyright 2000 by Schroff Development Corporation, Shawnee-Mission,        */
/* Kansas, United States of America. All rights reserved.                     */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* This Display Driver is distributed as "freeware". There are no             */
/* restrictions on its' usage.                                                */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* DISCLAIMER OF ALL WARRANTIES AND LIABILITY                                 */
/*                                                                            */
/* Schroff Development Corporation makes no warranties, either expressed      */
/* or implied, with respect to the programs contained within this file, or    */
/* with respect to software used in conjunction with these programs. The      */
/* programs are distributed 'as is'.  The entire risk as to its quality and   */
/* performance is assumed by the purchaser.  Schroff  Development Corporation */
/* does not guarantee, warrant or make any representation regarding the       */
/* use of, or the results of the use of the programs in terms of correctness, */
/* accuracy, reliability, or performance. Schroff Development Corporation     */
/* assumes no liability for any direct, indirect, or consquential, special    */
/* or exemplary damages, regardless of its having been advised of the         */
/* possibility of such damage.                                                */
/*                                                                            */
/******************************************************************************/


//
// This is a Display Driver that was written to comply with the PhotoRealistic
// RenderMan Display Driver Implementation Guide (on the web at:
// www.pixar.com/products/rendermandocs/toolkit/Toolkit/dspy.html).
//
// This driver places image data into a Windows .BMP file. It writes this data
// one scanline at a time and tries to minimize memory consumption.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include <aqsis/ri/ndspy.h>  // NOTE: Use Pixar's ndspy.h if you've got it.


typedef struct tagBITMAPFILEHEADER { // bmfh 
    short   bfType; 
    int     bfSize; 
    short   bfReserved1; 
    short   bfReserved2; 
    int     bfOffBits; 
} BITMAPFILEHEADER; 

/* instead of sizeof(BITMAPFILEHEADER) since this must be 14 */
#define BITMAPFILEHEADER_SIZEOF 14 

typedef struct tagRGBQUAD { // rgbq 
    unsigned char rgbBlue; 
    unsigned char rgbGreen; 
    unsigned char rgbRed; 
    unsigned char rgbReserved; 
} RGBQUAD; 

typedef struct tagBITMAPINFOHEADER{ // bmih 
    int     biSize; 
    long    biWidth; 
    long    biHeight; 
    short   biPlanes; 
    short   biBitCount;
    int     biCompression; 
    int     biSizeImage; 
    long    biXPelsPerMeter; 
    long    biYPelsPerMeter; 
    int     biClrUsed; 
    int     biClrImportant; 
} BITMAPINFOHEADER; 

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO;

#define BI_RGB 0

// -----------------------------------------------------------------------------
// Local structures
// -----------------------------------------------------------------------------

typedef struct
{
	// Bitmap data

	FILE              *fp;
	BITMAPFILEHEADER  bfh;
	char              *FileName;
	BITMAPINFO        bmi;
	char              *ImageData;
	int               Channels;
	int               RowSize;
	int               PixelBytes;
	long              TotalPixels;
}
AppData;


// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

static const int     DEFAULT_IMAGEWIDTH         = 512;   // Tiff display driver defaults are good enough for me.
static const int     DEFAULT_IMAGEHEIGHT        = 384;
static const float   DEFAULT_PIXELASPECTRATIO   = 1.0f;


//
// Set SHOW_CALLSTACK to 1 to see trace messages from the
// driver (which will be written to stderr).
//

#define SHOW_CALLSTACK 0

// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------

static int  DWORDALIGN(int bits);
static bool bitmapfileheader(BITMAPFILEHEADER *bfh, FILE *fp);
static unsigned short swap2( unsigned short s );
static unsigned long  swap4( unsigned long l );
static bool lowendian();

// -----------------------------------------------------------------------------
// Global Data
// -----------------------------------------------------------------------------


//******************************************************************************
// DspyImageOpen
//
// Initializes the display driver, allocates necessary resources, checks image
// size, specifies format in which incoming data will arrive.
//******************************************************************************

extern "C" PtDspyError DspyImageOpen(PtDspyImageHandle    *image,
                                     const char           *drivername,
                                     const char           *filename,
                                     int                  width,
                                     int                  height,
                                     int                  paramCount,
                                     const UserParameter  *parameters,
                                     int                  formatCount,
                                     PtDspyDevFormat      *format,
                                     PtFlagStuff          *flagstuff)
{
	PtDspyError rval = PkDspyErrorNone;

   static AppData g_Data;
   AppData *pData;

#if SHOW_CALLSTACK

	fprintf(stderr, "sdcBMP_DspyImageOpen called.\n");
#endif

   pData = (AppData *) calloc(1, sizeof(g_Data));
   *image = pData;

   // Initialize our global resources

	memset(&g_Data, 0, sizeof(AppData));

	flagstuff->flags = PkDspyFlagsWantsScanLineOrder;

	if ( width <= 0 )
		width = DEFAULT_IMAGEWIDTH;

	if ( height <= 0 )
		height = DEFAULT_IMAGEHEIGHT;

	
	g_Data.FileName = strdup(filename);
	g_Data.Channels = formatCount;

	g_Data.PixelBytes = 3; // One byte for red, one for green, and one for blue.

	g_Data.bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	g_Data.bmi.bmiHeader.biWidth       = width;
	g_Data.bmi.bmiHeader.biHeight      = height;
	g_Data.bmi.bmiHeader.biPlanes      = 1;
	g_Data.bmi.bmiHeader.biBitCount    = 24;
	g_Data.bmi.bmiHeader.biCompression = BI_RGB;

	g_Data.RowSize                     = DWORDALIGN(width * g_Data.bmi.bmiHeader.biBitCount);
	g_Data.bmi.bmiHeader.biSizeImage   = g_Data.RowSize * height;

	g_Data.TotalPixels     = width * height;


	// Prepare the file header

	g_Data.bfh.bfType     = 0x4D42;    // ASCII "BM"
	g_Data.bfh.bfSize     = BITMAPFILEHEADER_SIZEOF +
	                        sizeof(BITMAPINFOHEADER) +
	                        g_Data.bmi.bmiHeader.biSizeImage;
	g_Data.bfh.bfOffBits  = BITMAPFILEHEADER_SIZEOF + sizeof(BITMAPINFOHEADER);


	// Create a buffer for the image data (calloc clears all pixels to black)

	g_Data.ImageData = (char *)calloc(1, g_Data.RowSize);

	if ( ! g_Data.ImageData )
	{
		fprintf(stderr, "sdcBMP_DspyImageOpen_sdcBMP: Insufficient Memory\n");
		rval = PkDspyErrorNoResource;
	}


	// Open the file and get ready to write.

	g_Data.fp = fopen(g_Data.FileName, "wb");

	if ( ! g_Data.fp )
	{
		fprintf(stderr, "sdcBMP_DspyImageOpen: Unable to open [%s]\n", g_Data.FileName);
		rval = PkDspyErrorNoResource;
		goto Exit;
	}

	// Write out the BITMAPFILEHEADER
	if (lowendian())
	{
    		g_Data.bfh.bfType = swap2(g_Data.bfh.bfType);
    		g_Data.bfh.bfSize = swap4(g_Data.bfh.bfSize);
    		g_Data.bfh.bfOffBits = swap4(g_Data.bfh.bfOffBits);
	}
	if ( ! bitmapfileheader(&g_Data.bfh, g_Data.fp) )
      
	{
		fprintf(stderr, "sdcBMP_SaveBitmap: Error writing to [%s]\n", g_Data.FileName);
		goto Exit;
	}

	if (lowendian())
	{
    		g_Data.bfh.bfType = swap2(g_Data.bfh.bfType);
    		g_Data.bfh.bfSize = swap4(g_Data.bfh.bfSize);
    		g_Data.bfh.bfOffBits = swap4(g_Data.bfh.bfOffBits);
	}

	if (lowendian())
	{
   		g_Data.bmi.bmiHeader.biSize = swap4(g_Data.bmi.bmiHeader.biSize);
		   g_Data.bmi.bmiHeader.biWidth= swap4(g_Data.bmi.bmiHeader.biWidth);
   		g_Data.bmi.bmiHeader.biHeight= swap4(g_Data.bmi.bmiHeader.biHeight);
   		g_Data.bmi.bmiHeader.biPlanes = swap2(g_Data.bmi.bmiHeader.biPlanes);
   		g_Data.bmi.bmiHeader.biBitCount = swap2(g_Data.bmi.bmiHeader.biBitCount);
   		g_Data.bmi.bmiHeader.biCompression= swap4(g_Data.bmi.bmiHeader.biCompression); 
   		g_Data.bmi.bmiHeader.biSizeImage= swap4(g_Data.bmi.bmiHeader.biSizeImage);
   		g_Data.bmi.bmiHeader.biXPelsPerMeter= swap4(g_Data.bmi.bmiHeader.biXPelsPerMeter); 
   		g_Data.bmi.bmiHeader.biYPelsPerMeter= swap4(g_Data.bmi.bmiHeader.biYPelsPerMeter); 
   		g_Data.bmi.bmiHeader.biClrUsed= swap4(g_Data.bmi.bmiHeader.biClrUsed);
   		g_Data.bmi.bmiHeader.biClrImportant= swap4(g_Data.bmi.bmiHeader.biClrImportant);
	}
	// Write out the BITMAPINFOHEADER

	if ( ! fwrite(&g_Data.bmi.bmiHeader,
	              sizeof(BITMAPINFOHEADER),
	              1,
	              g_Data.fp) )
	{
		fprintf(stderr, "sdcBMP_SaveBitmap: Error writing to [%s]\n", g_Data.FileName);
		rval = PkDspyErrorNoResource;
		goto Exit;
	}

	if (lowendian())
	{
   		g_Data.bmi.bmiHeader.biSize = swap4(g_Data.bmi.bmiHeader.biSize);
		   g_Data.bmi.bmiHeader.biWidth= swap4(g_Data.bmi.bmiHeader.biWidth);
   		g_Data.bmi.bmiHeader.biHeight= swap4(g_Data.bmi.bmiHeader.biHeight);
   		g_Data.bmi.bmiHeader.biPlanes = swap2(g_Data.bmi.bmiHeader.biPlanes);
   		g_Data.bmi.bmiHeader.biBitCount = swap2(g_Data.bmi.bmiHeader.biBitCount);
   		g_Data.bmi.bmiHeader.biCompression= swap4(g_Data.bmi.bmiHeader.biCompression); 
   		g_Data.bmi.bmiHeader.biSizeImage= swap4(g_Data.bmi.bmiHeader.biSizeImage);
   		g_Data.bmi.bmiHeader.biXPelsPerMeter= swap4(g_Data.bmi.bmiHeader.biXPelsPerMeter); 
   		g_Data.bmi.bmiHeader.biYPelsPerMeter= swap4(g_Data.bmi.bmiHeader.biYPelsPerMeter); 
   		g_Data.bmi.bmiHeader.biClrUsed= swap4(g_Data.bmi.bmiHeader.biClrUsed);
   		g_Data.bmi.bmiHeader.biClrImportant= swap4(g_Data.bmi.bmiHeader.biClrImportant);
	}

   
   memcpy((void*) pData, (void *) &g_Data, sizeof(AppData));
   
Exit:

	if ( rval != PkDspyErrorNone )
	{
		if ( g_Data.fp )
			fclose(g_Data.fp);
		g_Data.fp = NULL;
	}

	return rval;
}



//******************************************************************************
// DspyImageQuery
//
// Query the display driver for image size (if not specified in the open call)
// and aspect ratio.
//******************************************************************************

extern "C" PtDspyError DspyImageQuery(PtDspyImageHandle image,
                                      PtDspyQueryType   type,
                                      size_t            size,
                                      void              *data)
{
#if SHOW_CALLSTACK
	fprintf(stderr, "sdcBMP_DspyImageQuery called, type: %d.\n", type);
#endif

   AppData *pData = (AppData *)image;
	PtDspyError          ret = PkDspyErrorNone;
	PtDspyOverwriteInfo  owi;
	PtDspySizeInfo       si;

	if ( size > 0 && data )
	{
		switch ( type )
		{
				case PkOverwriteQuery:

				if ( size > sizeof(owi) )
					size = sizeof(owi);

				owi.overwrite   = 1;
				owi.interactive = 0;

				memcpy(data, &owi, size);
				break;

				case PkSizeQuery:

				if ( size > sizeof(si) )
					size = sizeof(si);

				if ( image )
				{
					si.width       = pData->bmi.bmiHeader.biWidth;
					si.height      = pData->bmi.bmiHeader.biHeight;
					si.aspectRatio = DEFAULT_PIXELASPECTRATIO;
				}
				else
				{
					si.width       = DEFAULT_IMAGEWIDTH;
					si.height      = DEFAULT_IMAGEHEIGHT;
					si.aspectRatio = DEFAULT_PIXELASPECTRATIO;
				}

				memcpy(data, &si, size);
				break;

				default:
				ret = PkDspyErrorUnsupported;
				break;
		}
	}
	else
		ret = PkDspyErrorBadParams;

	return ret;
}




//******************************************************************************
// DspyImageData
//
// Send data to the display driver.
//******************************************************************************
extern "C" PtDspyError DspyImageData(PtDspyImageHandle image,
                          int xmin,
                          int xmax_plusone,
                          int ymin,
                          int ymax_plusone,
                          int entrysize,
                          const unsigned char *data)
{
	int  x;
	int  r, g, b;
     
	char *to;
	long spot;

   	AppData *pData = (AppData *)image;
   	r = g = b = 0;

#if SHOW_CALLSTACK

	fprintf(stderr, "sdcBMP_DspyImageData called.\n");
#endif

	if ( (ymin+1) != ymax_plusone )
	{
		fprintf(stderr, "sdcBMP_DspyImageData: Image data not in scanline format\n");
		return PkDspyErrorBadParams;
	}

	spot  = pData->bfh.bfOffBits +
	        (pData->RowSize * (pData->bmi.bmiHeader.biHeight - ymin - 1));
	spot += pData->PixelBytes * xmin;

	if ( fseek(pData->fp, spot, SEEK_SET) != 0 )
	{
		fprintf(stderr, "sdcBMP_DspyImageData: Seek failure\n");
		return PkDspyErrorUndefined;
	}


	to = pData->ImageData;

	for (x = xmin; x < xmax_plusone; x++)
	{
		// Extract the r, g, and b values from data

		if ( ! data )
			r = g = b = 0;
		else
			if ( pData->Channels == 1 )
				r = g = b = data[0];
			else
				if ( pData->Channels >= 3 )
				{
					r = data[pData->Channels - 1];
					g = data[pData->Channels - 2];
					b = data[pData->Channels - 3];
				}


		if ( data )
			data += entrysize;

		// Place the r, g, and b values into our bitmap

		*to++ = r;
		*to++ = g;
		*to++ = b;
	}

	if ( ! fwrite(pData->ImageData, int(to - pData->ImageData), 1, pData->fp) )
	{
		fprintf(stderr, "sdcBMP_DspyImageData: Error writing file\n");
		return PkDspyErrorUndefined;
	}

	return PkDspyErrorNone;
}



//******************************************************************************
// DspyImageClose
//******************************************************************************

extern "C" PtDspyError DspyImageClose(PtDspyImageHandle image)
{
#if SHOW_CALLSTACK
	fprintf(stderr, "sdcBMP_DspyImageClose called.\n");
#endif

   AppData *pData = (AppData *)image;

	if ( pData->fp )
		fclose(pData->fp);
	pData->fp = NULL;

	if ( pData->FileName )
		free(pData->FileName);
	pData->FileName = NULL;

	if ( pData->ImageData )
		free(pData->ImageData);
	pData->ImageData = NULL;

   free(pData);

	return PkDspyErrorNone;
}



//******************************************************************************
// DWORDALIGN
//******************************************************************************

static int DWORDALIGN(int bits)
{
	return int(((bits + 31) >> 5) << 2);
}

//******************************************************************************
// ExitApplication
//******************************************************************************
#if 0
static bool ExitApplication()
{
#if SHOW_CALLSTACK
	fprintf(stderr, "sdcBMP_ExitApplication called.\n");
#endif

	return true;
}
#endif

//******************************************************************************
// Save an header for bitmap; must be save field by field since the sizeof is 14
//******************************************************************************

static bool bitmapfileheader(BITMAPFILEHEADER *bfh, FILE *fp)
{
bool retval = true;


    retval = retval && (fwrite(&bfh->bfType, 1, 2, fp) == 2);
    retval = retval && (fwrite(&bfh->bfSize, 1, 4, fp) == 4);
    retval = retval && (fwrite(&bfh->bfReserved1, 1, 2, fp)== 2);
    retval = retval && (fwrite(&bfh->bfReserved2, 1, 2, fp)== 2);
    retval = retval && (fwrite(&bfh->bfOffBits, 1, 4, fp)== 4);
   

return retval;
}


//******************************************************************************
// Swap a short if you are not on NT/Pentium you must swap
//******************************************************************************
static unsigned short swap2( unsigned short s )
{
        unsigned short n;
        unsigned char *in, *out;

        out = ( unsigned char* ) & n;
        in = ( unsigned char* ) & s;

        out[ 0 ] = in[ 1 ];
        out[ 1 ] = in[ 0 ];
        return n;

}

//******************************************************************************
// Swap a long if you are not on NT/Pentium you must swap
//******************************************************************************
static unsigned long swap4(unsigned long l)
{
unsigned long n;
unsigned char *c, *d;

   c = (unsigned char*) &n;
   d = (unsigned char*) &l;


   c[0] = d[3];
   c[1] = d[2];
   c[2] = d[1];
   c[3] = d[0];
   return n;

}

//******************************************************************************
// Determine if we are low endian or big endian
//******************************************************************************

static bool lowendian()
{
 unsigned short low = 0xFFFE;
 unsigned char * pt = (unsigned char *) &low;

 return  (*pt == 0xFF); 
}
