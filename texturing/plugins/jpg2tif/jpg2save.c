#include "tiffio.h"

/*
 * save to filename a tiff file
 */
void save_tiff( char *filename, unsigned char *raster, int width, int length, int samples )
{
	/* save to a tiff file */
	int i;
	char version[ 80 ];
	unsigned char *pdata = raster;

	TIFF* ptex = TIFFOpen( filename, "w" );
	TIFFCreateDirectory( ptex );

	// Write the some form of version
	sprintf( version, "jpg2tif conversion for AQSIS" );

	TIFFSetField( ptex, TIFFTAG_SOFTWARE, ( char* ) version );
	TIFFSetField( ptex, TIFFTAG_IMAGEWIDTH, width );
	TIFFSetField( ptex, TIFFTAG_IMAGELENGTH, length );
	TIFFSetField( ptex, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
	TIFFSetField( ptex, TIFFTAG_BITSPERSAMPLE, 8 );
	TIFFSetField( ptex, TIFFTAG_SAMPLESPERPIXEL, samples );
	TIFFSetField( ptex, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
	TIFFSetField( ptex, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT );
	TIFFSetField( ptex, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS );
	TIFFSetField( ptex, TIFFTAG_ROWSPERSTRIP, 1 );


	for ( i = 0; i < length; i++ )
	{
		TIFFWriteScanline( ptex, pdata, i, 0 );
		pdata += ( width * samples );
	}
	TIFFWriteDirectory( ptex );
}
