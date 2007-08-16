#include <time.h>
#include "tiffio.h"

static char datetime[20];
/*
 * save to filename a tiff file
 */
extern 
#ifndef WIN32
inline
#endif
void save_tiff( char *filename,
                unsigned char *raster,
                int width,
                int length,
                int samples,
                char *conversion )
{
	/* save to a tiff file */
	int i;
	char version[ 80 ];
	unsigned char *pdata = raster;
	TIFF* ptex = TIFFOpen( filename, "w" );
	struct tm  *ct;
	int    year;
	int   bake = 0;
	int linewidth;

	time_t long_time;

	time( &long_time );           /* Get time as long integer. */
	ct = localtime( &long_time ); /* Convert to local time. */


	year=1900 + ct->tm_year;
	sprintf(datetime, "%04d:%02d:%02d %02d:%02d:%02d",
	        year, ct->tm_mon + 1, ct->tm_mday,
	        ct->tm_hour, ct->tm_min, ct->tm_sec);


	TIFFCreateDirectory( ptex );

	/* Write the some form of version */
	sprintf( version, "%s conversion for AQSIS", conversion );

	if (conversion && strstr(conversion, "bake2tif"))
		bake = 1;

	TIFFSetField( ptex, TIFFTAG_SOFTWARE, ( char* ) version );
	TIFFSetField( ptex, TIFFTAG_IMAGEWIDTH, width );
	TIFFSetField( ptex, TIFFTAG_IMAGELENGTH, length );
	TIFFSetField( ptex, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
	if (bake)
		TIFFSetField( ptex, TIFFTAG_BITSPERSAMPLE, 32 );
	else
		TIFFSetField( ptex, TIFFTAG_BITSPERSAMPLE, 8 );
	TIFFSetField( ptex, TIFFTAG_SAMPLESPERPIXEL, samples );
	TIFFSetField( ptex, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
	if (bake)
		TIFFSetField( ptex, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP );
	else
		TIFFSetField( ptex, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT );
	TIFFSetField( ptex, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS );
	TIFFSetField( ptex, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
	TIFFSetField( ptex, TIFFTAG_ROWSPERSTRIP, 1 );
	TIFFSetField( ptex, TIFFTAG_DATETIME, datetime);

	linewidth = width * samples;
	if (bake)
		linewidth *= sizeof(float);
	for ( i = 0; i < length; i++ )
	{
		TIFFWriteScanline( ptex, pdata, i, 0 );
		pdata += linewidth;
	}
	if (!bake)
		TIFFWriteDirectory( ptex );
	TIFFClose( ptex );
}
