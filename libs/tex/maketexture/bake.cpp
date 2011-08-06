// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/** \file
 *
 * \brief Conversion utilities from 2D point clouds to a baked texture
 *
 * \author Michel Joron (?)
 * \author Chris Foster [chris42f (at) gmail (d0t) com]
 */

#include "bake.h"

#include <ctime>
#include <cstdio>
#include <cstdlib>

#include <aqsis/math/math.h>
#include <aqsis/util/logging.h>
#include <aqsis/tex/texexception.h>
#include "tiffio.h"

namespace Aqsis {

namespace {

/** \todo Remove
 * save to filename a tiff file
 */
void save_tiff( const char *filename,
				TqFloat *raster,
				TqInt width,
				TqInt length,
				TqInt samples,
				const char* description)
{
	/* save to a tiff file */
	TqInt i;
	TqUchar *pdata = (TqUchar *) raster;
	TIFF* ptex = TIFFOpen( filename, "w" );
	static TqChar datetime[20];
	struct tm  *ct;
	TqInt    year;
	TqInt linewidth;

	time_t long_time;

	time( &long_time );           /* Get time as long integer. */
	ct = localtime( &long_time ); /* Convert to local time. */


	year=1900 + ct->tm_year;
	std::sprintf(datetime, "%04d:%02d:%02d %02d:%02d:%02d",
	        year, ct->tm_mon + 1, ct->tm_mday,
	        ct->tm_hour, ct->tm_min, ct->tm_sec);


	TIFFCreateDirectory( ptex );


	TIFFSetField( ptex, TIFFTAG_SOFTWARE, description );
	TIFFSetField( ptex, TIFFTAG_IMAGEWIDTH, width );
	TIFFSetField( ptex, TIFFTAG_IMAGELENGTH, length );
	TIFFSetField( ptex, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
	TIFFSetField( ptex, TIFFTAG_BITSPERSAMPLE, 32 );
	TIFFSetField( ptex, TIFFTAG_SAMPLESPERPIXEL, samples );
	TIFFSetField( ptex, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
	TIFFSetField( ptex, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP );
	TIFFSetField( ptex, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS );
	TIFFSetField( ptex, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
	TIFFSetField( ptex, TIFFTAG_ROWSPERSTRIP, 1 );
	TIFFSetField( ptex, TIFFTAG_DATETIME, datetime);

	linewidth = width * samples;
	linewidth *= sizeof(TqFloat);
	for ( i = 0; i < length; i++ )
	{
		TIFFWriteScanline( ptex, pdata, i, 0 );
		pdata += linewidth;
	}
	TIFFClose( ptex );
}

/// \todo Remove duplication somehow?
TqFloat	RiDiskFilter(TqFloat x, TqFloat y, TqFloat xwidth, TqFloat ywidth)
{
	double d, xx, yy;

	xx = x * x;
	yy = y * y;
	xwidth *= 0.5;
	ywidth *= 0.5;

	d = ( xx ) / ( xwidth * xwidth ) + ( yy ) / ( ywidth * ywidth );
	if ( d < 1.0 )
	{
		return 1.0;
	}
	else
	{
		return 0.0;
	}
}

/// \todo Remove these?
const float filter_width = 8;
const int filter_size = 16;

} // unnamed namespace


void bakeToTiff(const char* in, const char* tiffname, int bake)
{
	FILE * bakefile;

	TqUshort h, w;
	TqFloat s, t, r1, g1, b1;
	TqFloat *pixels;
	TqFloat *xpixels;
	TqFloat *filter, *pf;
	TqChar buffer[ 200 ];
	TqInt i, k, n;
	TqInt x, y;
	TqFloat mins, mint, maxs, maxt;
	bool normalized = true;
	TqInt count, number;
	TqFloat *temporary;
	TqInt elmsize = 3;
	TqFloat invWidth, invSize;

	count = 1024 * 1024;
	number = 0;


	h = w = bake;
	pixels = ( TqFloat * ) calloc( 4, bake * bake * sizeof(TqFloat));
	temporary = (TqFloat *) malloc(count *  5 * sizeof(TqFloat));

	bakefile = fopen( in, "rb" );

	/* Ignore the first line of text eg. Aqsis bake file */
	if(std::fgets( buffer, 200, bakefile ) == NULL)
		AQSIS_THROW_XQERROR(XqInternal, EqE_BadFile, "Could not read bake file header");
	// Read the second line of text to configure how many floats we expect to
	// read
	if(std::fgets( buffer, 200, bakefile ) == NULL || std::sscanf(buffer, "%d", &elmsize) == 0)
		AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile, "Could not read length of bake file \"" << in << "\"");

	while ( std::fgets( buffer, 200, bakefile ) != NULL )
	{
		k = number * 5;

		switch (elmsize)
		{
			case 3:
			{
				sscanf( buffer, "%f %f %f %f %f", &s, &t, &r1, &g1, &b1 );
			}
			break;
			case 2:
			{
				sscanf( buffer, "%f %f %f %f", &s, &t, &r1, &g1);
				b1 = (r1 + g1) / 2.0;
			}
			break;
			default:
			case 1:
			{
				sscanf( buffer, "%f %f %f", &s, &t, &r1);
				g1 = b1 = r1;
			}
			break;
		}
		temporary[k] = s;
		temporary[k+1] = t;
		temporary[k+2] = r1;
		temporary[k+3] = g1;
		temporary[k+4] = b1;

		number++;
		if (number >= (count - 1))
		{
			count += 1024;
			temporary = (float *)  realloc((void *) temporary, count * 5 * sizeof(float));
		}
	}

	/* printf("done\nFind the max, min of s,t.\n"); */
	mins = maxs = temporary[0];
	mint = maxt = temporary[1];

	/* Find the min,max of s and t */
	for (i=0; i < number; i++)
	{
		k = i * 5;

		if (mins > temporary[k])
			mins = temporary[k];
		if (mint > temporary[k+1])
			mint = temporary[k+1];
		if (maxs < temporary[k])
			maxs = temporary[k];
		if (maxt < temporary[k+1])
			maxt = temporary[k+1];
	}


	if ((mins >= 0.0 && maxs <= 1.0) &&
	        (maxt >= 0.0 && maxt <= 1.0) )
	{
		normalized = false;
	}

	if (normalized == true)
	{
		Aqsis::log() << "bake2tif normalizes the keys (normally s,t)" << std::endl;
		Aqsis::log() << "\t(min_s, max_s): (" << mins << ", " << maxs << "), (min_t, max_t): (" << mint << ", " << maxt << ")" << std::endl;
	}

	/* Try to adjust with the final resolution of mipmap */
	/* filter_size = max((int) log((double)bake)/log(2.0) + 2, (int) FILTER_TBL_SIZE);
	 * filter_size = (int) ceil(log((double)min((int) ( (maxs - mins) * bake), (int) ((maxt -mint ) * bake)))/log(2.0));
	     */
	invWidth = 1.0f/filter_width;
	invSize = 1.0f/filter_size;

	/* init the filter' table */
	filter = (TqFloat *) calloc(filter_size*filter_size,sizeof(TqFloat));
	pf = filter;
	for (y=0; y < filter_size; ++y)
	{
		TqFloat fy = (TqFloat)( y+ 0.5f) * filter_width * invSize;
		for (x=0; x < filter_size; ++x)
		{
			TqFloat fx = ((TqFloat) x+ 0.5f) * filter_width * invSize;
			/* we will use a disk filter the point will dispose in
			 * a circle around each point; more pleasant visually
			 */
			*pf++ = RiDiskFilter(fx,fy,filter_width, filter_width);
		}
	}


	/* Now it is time to save s,t, r1, g1, b1 into pixels array (along with the sum/area filtering accumalor  */
	for (i=0; i < number; i++)
	{
		TqInt x0, x1, y0, y1;
		TqInt *ifx, *ify;
		TqFloat dImageX;
		TqFloat dImageY;

		k = i * 5;


		s = temporary[k];
		t = temporary[k+1];
		r1 = temporary[k+2];
		g1 = temporary[k+3];
		b1 = temporary[k+4];

		/* printf("%d\n", number);  */

		/* Normalize the s,t between 0..1.0  only if required
		               */
		if (normalized)
		{
			if ( (maxs - mins) != 0.0)
			{
				s = (s - mins) / (maxs - mins);
			}
			else
			{
				if (s < 0.0) s *= -1.0;
				if (s > 1.0) s = 1.0;
			}
			if ((maxt - mint) != 0.0)
			{
				t = (t - mint) / (maxt - mint);
			}
			else
			{
				if (t < 0.0) t *= -1.0;
				if (t > 1.0) t = 1.0;
			}
		}
		/* When we have some collision ? What should be nice
		 * to accumulate the RGB values instead ?
		 */
		x = (int)( s * ( bake - 1 ) );
		y = (int)( t * ( bake - 1 ) );

		/* printf("x %d y %d rgb %f %f %f\n", x, y, r1, g1, b1); */

		/* each each pixels accumulated it in xpixels but
		* make sure we use
		* a filtering of 16x16 to garantee spreading of the
		* values across x,y pixels.
		*/

		dImageX = x - 0.5f;
		dImageY = y - 0.5f;

		x0 = (TqInt) ceil(dImageX - filter_width);
		x1 = (TqInt) floor(dImageX + filter_width);
		y0 = (TqInt) ceil(dImageY - filter_width);
		y1 = (TqInt) floor(dImageY + filter_width);

		x0 = max(x0, 0);
		x1 = min(x1, bake -1);
		y0 = max(y0, 0);
		y1 = min(y1, bake -1);

		if ( ( (x1-x0)<0) || ((y1-y0)<0 )) continue;

		/* filter delta indexes*/
		ifx = (TqInt*)calloc(x1-x0+1, sizeof(TqInt));
		for (x = x0; x <= x1; ++x)
		{
			TqFloat fx = fabsf(x -dImageX) * invWidth * filter_size;
			ifx[x-x0] = min((int) floor(fx), filter_size - 1);
		}
		ify = (TqInt*)calloc(y1-y0+1, sizeof(TqInt));
		for (y = y0; y <= y1; ++y)
		{
			TqFloat fy = fabsf(y -dImageY) * invWidth * filter_size;
			ify[y-y0] = min((int) floor(fy), filter_size - 1);
		}

		/* Fill all the right pixels now */
		for (y = y0; y < y1; ++y)
		{
			for (x = x0; x <= x1; ++x)
			{
				TqInt offset;
				TqFloat filterWt;
				offset = ify[y-y0]*filter_size + ifx[x-x0];
				/* printf("offset %d ", offset); */
				filterWt = filter[offset];

				/* Remove the negative lob maybe ?
				 * if (filterWt < 0.0) continue;
				 */

				/* printf("wt %f\n", filterWt); */
				n = (y * bake + x);
				n *= 4;
				pixels[n] += (filterWt * r1);
				pixels[n+1] += (filterWt * g1);
				pixels[n+2] += (filterWt * b1);
				pixels[n+3] += filterWt;
				/* printf("x %d y %d rgb wt %f %f %f %f\n", x, y, pixels[n], pixels[n+1], pixels[n+2], pixels[n+3]); */
			}
		}


		free(ifx);
		free(ify);
	}


	/* Now it is time to unroll the filterWt and save into xpixels */

	xpixels = ( TqFloat * ) calloc( 3, bake * bake * sizeof(TqFloat));
	//CqTextureBuffer<TqFloat> xpixels(bake, bake, 3);

	for (y=0; y < bake; ++y)
	{
		for (x=0; x < bake; ++x)
		{
			TqInt m;
			n = (y * bake + x);
			m = n;
			m *= 3;
			n *= 4;
			if (pixels[n+3] > 0.0)
			{
				/* printf("unroll weightSum factor \n"); */
				float area = 1.0/pixels[n+3];
				xpixels[m] =  pixels[n] * area;
				xpixels[m+1] =  pixels[n+1] * area;
				xpixels[m+2] =  pixels[n+2] * area;
			}
		}
	}

	save_tiff( tiffname, xpixels, w, h, 3, "Aqsis Renderer, bake2tif conversion");

	free( pixels );
	free( xpixels );
	free( filter );
	free(temporary);

	fclose( bakefile );

}

} // namespace Aqsis
