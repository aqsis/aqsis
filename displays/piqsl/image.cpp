// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
		\brief Implements the basic image functionality.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"
#include "version.h"

#include "image.h"
#include "framebuffer.h"
#include "logging.h"
#include "ndspy.h"

#include <tiffio.h>

START_NAMESPACE( Aqsis )

CqImage::~CqImage()
{
	free(m_data);
	free(m_realData);
}

void CqImage::PrepareImageBuffer()
{
	//boost::mutex::scoped_lock lock(mutex());
	m_data = reinterpret_cast<unsigned char*>(malloc( m_imageWidth * m_imageHeight * numChannels() * sizeof(TqUchar)));
	// Initialise the display to a checkerboard to show alpha
	for (TqUlong i = 0; i < imageHeight(); i ++)
	{
		for (TqUlong j = 0; j < imageWidth(); j++)
		{
			int     t       = 0;
			TqUchar d = 255;

			if ( ( (imageHeight() - 1 - i) & 31 ) < 16 )
				t ^= 1;
			if ( ( j & 31 ) < 16 )
				t ^= 1;
			if ( t )
				d      = 128;
			for(TqUint chan = 0; chan < numChannels(); chan++)
				data()[numChannels() * (i*imageWidth() + j) + chan ] = d;
		}
	}
	// Now prepare the buffer for the natural data.
	// First work out how big each element is by scanning the channels specification.
	m_elementSize = 0;
	for(std::vector<std::pair<std::string, TqInt> >::iterator channel = m_channels.begin(); channel != m_channels.end(); ++channel)
	{
		switch(channel->second)
		{
			case PkDspyFloat32:
			case PkDspyUnsigned32:
			case PkDspySigned32:
				m_elementSize += 4;
				break;
			case PkDspyUnsigned16:
			case PkDspySigned16:
				m_elementSize += 2;
				break;
			case PkDspyUnsigned8:
			case PkDspySigned8:
			default:
				m_elementSize += 1;
				break;
		}
	}
	m_realData = reinterpret_cast<unsigned char*>(malloc( m_imageWidth * m_imageHeight * m_elementSize));
}

void CqImage::setUpdateCallback(boost::function<void(int,int,int,int)> f)
{
	m_updateCallback = f;
}

TiXmlElement* CqImage::serialiseToXML()
{
	TiXmlElement* imageXML = new TiXmlElement("Image");

	TiXmlElement* typeXML = new TiXmlElement("Type");
	TiXmlText* typeText = new TiXmlText("external");
	typeXML->LinkEndChild(typeText);
	imageXML->LinkEndChild(typeXML);

	TiXmlElement* nameXML = new TiXmlElement("Name");
	TiXmlText* nameText = new TiXmlText(name());
	nameXML->LinkEndChild(nameText);
	imageXML->LinkEndChild(nameXML);

	TiXmlElement* filenameXML = new TiXmlElement("Filename");
	TiXmlText* filenameText = new TiXmlText(filename());
	filenameXML->LinkEndChild(filenameText);
	imageXML->LinkEndChild(filenameXML);

	return(imageXML);
}

void CqImage::saveToTiff(const std::string& filename)
{
	uint16 config = PLANARCONFIG_CONTIG;
	struct tm *ct;
	char mydescription[80];
	int year;

	time_t long_time;

	time( &long_time );           /* Get time as long integer. */
	ct = localtime( &long_time ); /* Convert to local time. */

	year=1900 + ct->tm_year;
	char datetime[21];
	sprintf(datetime, "%04d:%02d:%02d %02d:%02d:%02d", year, ct->tm_mon + 1,
	        ct->tm_mday, ct->tm_hour, ct->tm_min, ct->tm_sec);

#if 0
	if(description.empty())
	{
		double nSecs = difftime(long_time, start);
		sprintf(mydescription,"%d secs", static_cast<TqInt>(nSecs));
		start = long_time;
	}
	else
	{
		strcpy(mydescription, description.c_str());
	}
#endif


	// Set common tags
	// If in "shadowmap" mode, write as a shadowmap.
#if 0
	if( image->m_imageType == Type_Shadowmap )
	{
		SaveAsShadowMap(filename, image, mydescription);
		return;
	}
	else if( image->m_imageType == Type_ZFile )
	{
		std::ofstream ofile( filename.c_str(), std::ios::out | std::ios::binary );
		if ( ofile.is_open() )
		{
			// Save a file type and version marker
			ofile << ZFILE_HEADER;

			// Save the xres and yres.
			ofile.write( reinterpret_cast<char* >( &image->m_width ), sizeof( image->m_width ) );
			ofile.write( reinterpret_cast<char* >( &image->m_height ), sizeof( image->m_height ) );

			// Save the transformation matrices.
			ofile.write( reinterpret_cast<char*>( image->m_matWorldToCamera[ 0 ] ), sizeof( image->m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<char*>( image->m_matWorldToCamera[ 1 ] ), sizeof( image->m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<char*>( image->m_matWorldToCamera[ 2 ] ), sizeof( image->m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<char*>( image->m_matWorldToCamera[ 3 ] ), sizeof( image->m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );

			ofile.write( reinterpret_cast<char*>( image->m_matWorldToScreen[ 0 ] ), sizeof( image->m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<char*>( image->m_matWorldToScreen[ 1 ] ), sizeof( image->m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<char*>( image->m_matWorldToScreen[ 2 ] ), sizeof( image->m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<char*>( image->m_matWorldToScreen[ 3 ] ), sizeof( image->m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );

			// Now output the depth values
			ofile.write( reinterpret_cast<char*>( image->m_data ), sizeof( TqFloat ) * ( image->m_width * image->m_height ) );
			ofile.close();
		}
		return;
	}
#endif

	TIFF* pOut = TIFFOpen( filename.c_str(), "w" );

	if ( pOut )
	{
		// Write the image to a tiff file.
		char version[ 80 ];

		short ExtraSamplesTypes[ 1 ] = {EXTRASAMPLE_ASSOCALPHA};

		sprintf( version, "%s %s (%s %s)", STRNAME, VERSION_STR, __DATE__, __TIME__);

		TIFFSetField( pOut, TIFFTAG_SOFTWARE, ( char* ) version );
		TIFFSetField( pOut, TIFFTAG_IMAGEWIDTH, ( uint32 ) imageWidth() );
		TIFFSetField( pOut, TIFFTAG_IMAGELENGTH, ( uint32 ) imageHeight() );
		TIFFSetField( pOut, TIFFTAG_XRESOLUTION, (float) 1.0 );
		TIFFSetField( pOut, TIFFTAG_YRESOLUTION, (float) 1.0 );
		TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, (short) 8 );
		//TIFFSetField( pOut, TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, image->m_matWorldToCamera );
		//TIFFSetField( pOut, TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, image->m_matWorldToScreen );
		TIFFSetField( pOut, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
		TIFFSetField( pOut, TIFFTAG_SAMPLESPERPIXEL, numChannels() );
		TIFFSetField( pOut, TIFFTAG_DATETIME, datetime);
//		if (!image->m_hostname.empty())
//			TIFFSetField( pOut, TIFFTAG_HOSTCOMPUTER, image->m_hostname.c_str() );
		TIFFSetField( pOut, TIFFTAG_IMAGEDESCRIPTION, mydescription);

		// Set the position tages in case we aer dealing with a cropped image.
		TIFFSetField( pOut, TIFFTAG_XPOSITION, ( float ) originX() );
		TIFFSetField( pOut, TIFFTAG_YPOSITION, ( float ) originY() );
		TIFFSetField( pOut, TIFFTAG_PLANARCONFIG, config );
//		TIFFSetField( pOut, TIFFTAG_COMPRESSION, image->m_compression );
//		if ( image->m_compression == COMPRESSION_JPEG )
//			TIFFSetField( pOut, TIFFTAG_JPEGQUALITY, image->m_quality );
		TIFFSetField( pOut, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );

		if ( numChannels() == 4 )
			TIFFSetField( pOut, TIFFTAG_EXTRASAMPLES, 1, ExtraSamplesTypes );

		TIFFSetField( pOut, TIFFTAG_SAMPLESPERPIXEL, numChannels() );
		TIFFSetField( pOut, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize( pOut, 0 ) );

		TqInt lineLength = elementSize() * imageWidth();

		// Work out the format of the image to write.
		TqInt widestType = PkDspyUnsigned8;
		for(TqInt ichan = 0; ichan < numChannels(); ++ichan)
			widestType = std::min(widestType, channelType(ichan));

		// Write out an 8 bits per pixel integer image.
		if ( widestType == PkDspyUnsigned8 || widestType == PkDspySigned8 )
		{
			TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT );
			TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 8 );
		}
		else if(widestType == PkDspyFloat32)
		{
			/* use uncompressed IEEEFP pixels */
			TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP );
			TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 32 );
		}
		else if(widestType == PkDspySigned16 || widestType == PkDspyUnsigned16)
		{
			TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT );
			TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 16 );
		}
		else if(widestType == PkDspySigned32 || widestType == PkDspyUnsigned32)
		{
			TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT );
			TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 32 );
		}
		TqInt row;
		for ( row = 0; row < imageHeight(); row++ )
		{
			if ( TIFFWriteScanline( pOut, reinterpret_cast<void*>(reinterpret_cast<char*>(m_realData) + ( row * lineLength ))
									, row, 0 ) < 0 )
				break;
		}
		TIFFClose( pOut );
	}
}

// Simple image loading, uses TIFFReadRGBAImage to hide
// format details. May be better in the future to read
// the image at a lower level to maintain the native format.
void CqImage::loadFromTiff(const std::string& filename)
{
	const char* defChannelNames[] = {
		"r",
		"g",
		"b",
		"a"
	};

	TIFF* tif = TIFFOpen(filename.c_str(), "r");
	if (tif) 
	{
		uint32 w, h;
		uint16 nChannels;
		uint16 bitsPerSample;
		uint16 sampleFormat;

		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &nChannels);
		TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
		TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sampleFormat);
		// \todo: Should read the origin and frame size out of the image.
		setOrigin(0,0);
		setImageSize(w, h);
		setFrameSize(w, h);
		Aqsis::log() << Aqsis::info << "Loading image " << filename << std::endl;
		m_channels.clear();
		for(TqUint channel = 0; channel < 4; ++channel)
			addChannel(defChannelNames[channel], PkDspyUnsigned8);
		PrepareImageBuffer();

		if(!TIFFIsTiled(tif))
		{
			tdata_t buf;
			uint32 row;
			uint16 config;

			TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);
			buf = _TIFFmalloc(TIFFScanlineSize(tif));
			TqUlong localOffset = 0;
			if (config == PLANARCONFIG_CONTIG) 
			{
				for (row = 0; row < h; row++)
				{
					TIFFReadScanline(tif, buf, row);
					memcpy(m_realData+localOffset, buf, rowLength());
					localOffset+=rowLength();
				}
			} 
			else if (config == PLANARCONFIG_SEPARATE) 
			{
				#if 0
				uint16 s, nsamples;

				TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &nsamples);
				for (s = 0; s < nsamples; s++)
				{
					for (row = 0; row < imagelength; row++)
					{
						TIFFReadScanline(tif, buf, row, s);
						memcpy(m_realData+localOffset, buf, rowLength());
						localOffset+=rowLength();
					}
				}
				#else
				Aqsis:log() << Aqsis::error << "Images with separate planar config nto supported" << std::endl;
				#endif
			}
			_TIFFfree(buf);
		}
		else
			Aqsis::log() << Aqsis::error << "Cannot currently load images in tiled format." << std::endl;
//		TIFFReadRGBAImage(tif, w, h, (uint32*)m_data, 0);
		TIFFClose(tif);

		setFilename(filename);
		transferData();

		// As the standard TIFF images are inverted compared with our internal representation,
		// we need to do a flip.
		TqUlong linelength = w * sizeof(uint32);
		unsigned char* temp = new unsigned char[linelength];
		TqUlong topline = 0;
		TqUlong bottomline = h-1;
		while(topline < bottomline)
		{
			memcpy(temp, m_data+(linelength*topline), linelength);
			memcpy(m_data+(linelength*topline), m_data+(linelength*bottomline), linelength);
			memcpy(m_data+(linelength*bottomline), temp, linelength);
			topline++;
			bottomline--;
		}
		delete[](temp);
    }
}

void CqImage::transferData()
{
	assert(elementSize == m_elementSize);

	boost::mutex::scoped_lock lock(mutex());

	TqUlong y;
	unsigned char *unrolled = m_data;
	unsigned char *realData = m_realData;

	for ( y = 0; y < imageHeight(); y++ )
	{
		TqUlong x;
		for ( x = 0; x < imageWidth(); x++ )
		{
			TqInt displayOffset = numChannels() * (( y * imageWidth() ) +  x );
			TqInt storageOffset = (( y * rowLength() ) + ( x * m_elementSize ) );
			std::vector<std::pair<std::string, TqInt> >::iterator channel;
			for(channel = m_channels.begin(); channel != m_channels.end(); ++channel)
			{
				switch(channel->second)
				{
					case PkDspyUnsigned16:
						unrolled[displayOffset] = reinterpret_cast<TqUshort*>(&realData[storageOffset])[0]>>8;
						break;
					case PkDspySigned16:
						unrolled[displayOffset] = reinterpret_cast<TqShort*>(&realData[storageOffset])[0]>>7;
						break;
					case PkDspyUnsigned32:
						unrolled[displayOffset] = reinterpret_cast<TqUlong*>(&realData[storageOffset])[0]>>24;
						break;
					case PkDspySigned32:
						unrolled[displayOffset] = reinterpret_cast<TqLong*>(&realData[storageOffset])[0]>>23;
						break;

					case PkDspyFloat32:
						unrolled[displayOffset] = reinterpret_cast<TqFloat*>(&realData[storageOffset])[0]*255.0;
						break;

					case PkDspySigned8:
					case PkDspyUnsigned8:
					default:
						unrolled[displayOffset] = reinterpret_cast<TqUchar*>(&realData[storageOffset])[0];
						break;
				}
				++displayOffset;
				++storageOffset;
			}
		}
	}
	if(m_updateCallback)
		m_updateCallback(-1, -1, -1, -1);
}

END_NAMESPACE( Aqsis )
