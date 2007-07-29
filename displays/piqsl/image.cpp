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

#include "aqsis.h"

#include <tiffio.h>
#include <ctime>

#include "version.h"
#include "image.h"
#include "framebuffer.h"
#include "logging.h"
#include "ndspy.h"


START_NAMESPACE( Aqsis )

CqImage::~CqImage()
{
}

void CqImage::PrepareImageBuffer()
{
	//boost::mutex::scoped_lock lock(mutex());
	m_data = boost::shared_array<unsigned char>(new unsigned char[( m_imageWidth * m_imageHeight * numChannels() )]);
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
	// Work out how big each element is by scanning the channel specification.
	m_elementSize = CqImageBuffer::bytesPerPixel(m_channels);
	// Now prepare a buffer for the natural data.
	m_realData = boost::shared_array<unsigned char>(new unsigned char[( m_imageWidth * m_imageHeight * m_elementSize)]);
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
		//TIFFSetField( pOut, TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, image->m_matWorldToCamera );
		//TIFFSetField( pOut, TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, image->m_matWorldToScreen );
		TIFFSetField( pOut, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
		TIFFSetField( pOut, TIFFTAG_SAMPLESPERPIXEL, numChannels() );
		TIFFSetField( pOut, TIFFTAG_DATETIME, datetime);
//		if (!image->m_hostname.empty())
//			TIFFSetField( pOut, TIFFTAG_HOSTCOMPUTER, image->m_hostname.c_str() );
		TIFFSetField( pOut, TIFFTAG_IMAGEDESCRIPTION, mydescription);

		setDescription(std::string(mydescription));

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
		TqUint widestType = PkDspyUnsigned8;
		for(TqUint ichan = 0; ichan < numChannels(); ++ichan)
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
		else if(widestType == PkDspySigned16)
		{
			TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT );
			TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 16 );
		}
		else if(widestType == PkDspyUnsigned16)
		{
			TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT );
			TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 16 );
		}
		else if(widestType == PkDspySigned32)
		{
			TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT );
			TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 32 );
		}
		else if(widestType == PkDspyUnsigned32)
		{
			TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT );
			TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 32 );
		}
		TqUint row;
		for ( row = 0; row < imageHeight(); row++ )
		{
			if ( TIFFWriteScanline( pOut, reinterpret_cast<void*>(m_realData.get() + ( row * lineLength ))
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
		uint16 nChannels = 1;
		uint16 bitsPerSample = 1;
		uint16 sampleFormat = SAMPLEFORMAT_UINT;
		TqInt internalFormat = PkDspyUnsigned8;

		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &nChannels);
		TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
		TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sampleFormat);
		TqChar *description = "";
		if (TIFFGetField(tif, TIFFTAG_IMAGEDESCRIPTION, &description) != 1)
		{
			TIFFGetField(tif, TIFFTAG_SOFTWARE, &description);
		}
		switch(sampleFormat)
		{
			case SAMPLEFORMAT_UINT:
			{
				switch(bitsPerSample)
				{
					case 8:
						internalFormat = PkDspyUnsigned8;
						break;
					case 16:
						internalFormat = PkDspyUnsigned16;
						break;
					case 32:
						internalFormat = PkDspyUnsigned32;
						break;
					default:
						Aqsis::log() << Aqsis::error << "Unrecognised bit depth for unsigned int format " << bitsPerSample << std::endl;
						return;
				}
			}
			break;

			case SAMPLEFORMAT_INT:
			{
				switch(bitsPerSample)
				{
					case 8:
						internalFormat = PkDspySigned8;
						break;
					case 16:
						internalFormat = PkDspySigned16;
						break;
					case 32:
						internalFormat = PkDspySigned32;
						break;
					default:
						Aqsis::log() << Aqsis::error << "Unrecognised bit depth for signed int format " << bitsPerSample << std::endl;
						return;
				}
			}
			break;

			case SAMPLEFORMAT_IEEEFP:
			{
				if(bitsPerSample != 32)
				{
					Aqsis::log() << Aqsis::error << "Unrecognised bit depth for ieeefp format " << bitsPerSample << std::endl;
					return;
				}
				internalFormat = PkDspyFloat32;
			}
			break;

			default:
				Aqsis::log() << Aqsis::error << "Unrecognised format " << sampleFormat << std::endl;
				return;
		}

		// \todo: Should read the origin and frame size out of the image.
		setOrigin(0,0);
		setImageSize(w, h);
		setFrameSize(w, h);
		setDescription(std::string(description));
		Aqsis::log() << Aqsis::info << "Loading image " << filename << " [" << h << "x" << w << "x" << nChannels << "] (" << internalFormat << ")" << std::endl;
		m_channels.clear();
		for(TqUint channel = 0; channel < nChannels; ++channel)
			addChannel(defChannelNames[channel], internalFormat);
		PrepareImageBuffer();

		if(!TIFFIsTiled(tif))
		{
			uint32 row;
			uint16 config;

			TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);
			boost::shared_ptr<void> buf(_TIFFmalloc(TIFFScanlineSize(tif)), _TIFFfree);
			TqUlong localOffset = 0; 
			if (config == PLANARCONFIG_CONTIG) 
			{
				for (row = 0; row < h; row++)
				{
					TIFFReadScanline(tif, buf.get(), row);
					_TIFFmemcpy(m_realData.get()+localOffset, buf.get(), rowLength());
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
				Aqsis::log() << Aqsis::error << "Images with separate planar config not supported." << std::endl;
				#endif
			}
		}
		else
			Aqsis::log() << Aqsis::error << "Cannot currently load images in tiled format." << std::endl;
//		TIFFReadRGBAImage(tif, w, h, (uint32*)m_data, 0);
		TIFFClose(tif);

		setFilename(filename);

		boost::mutex::scoped_lock lock(mutex());
		CqImageBuffer::quantizeForDisplay(m_realData.get(), m_data.get(), m_channels, m_imageWidth, m_imageHeight);
		if(m_updateCallback)
			m_updateCallback(-1, -1, -1, -1);
    }
}


END_NAMESPACE( Aqsis )
