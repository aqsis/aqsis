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
		\brief Implements an image class getting it's data from the Dspy server.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/


#include	"aqsis.h"
#include	"file.h"

#include	<fstream>
#include	<map>
#include	<algorithm>
#include	"signal.h"

#ifdef AQSIS_SYSTEM_WIN32

#include	<process.h>

#else // AQSIS_SYSTEM_WIN32

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>

static const int INVALID_SOCKET = -1;
static const int SD_BOTH = 2;
static const int SOCKET_ERROR = -1;

typedef sockaddr_in SOCKADDR_IN;
typedef sockaddr* PSOCKADDR;

#endif // !AQSIS_SYSTEM_WIN32

#include	"renderer.h"
#include	"render.h"
#include	"displayserverimage.h"

#include <tiffio.h>

#include "boost/archive/iterators/base64_from_binary.hpp"
#include "boost/archive/iterators/transform_width.hpp"
#include "boost/archive/iterators/insert_linebreaks.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

START_NAMESPACE( Aqsis )


//---------------------------------------------------------------------
/** Close the socket this client is associated with.
 */
void CqDisplayServerImage::close()
{
	m_socket.close();
}


//----------------------------------------------------------------------
/** CompositeAlpha() Composite with the alpha the end result RGB
*
*/

#define INT_MULT(a,b,t) ( (t) = (a) * (b) + 0x80, ( ( ( (t)>>8 ) + (t) )>>8 ) )
#define INT_PRELERP(p, q, a, t) ( (p) + (q) - INT_MULT( a, p, t) )

void CompositeAlpha(TqInt r, unsigned char &R, unsigned char alpha )
{ 
	TqInt t;
	// C’ = INT_PRELERP( A’, B’, b, t )
	TqInt R1 = static_cast<TqInt>(INT_PRELERP( R, r, alpha, t ));
	R = CLAMP( R1, 0, 255 );
}


void CqDisplayServerImage::acceptData(TqUlong xmin, TqUlong xmaxplus1, TqUlong ymin, TqUlong ymaxplus1, TqInt elementSize, const unsigned char* bucketData )
{
	assert(elementSize == m_elementSize);

	TqUlong xmin__ = MAX((xmin - originX()), 0);
	TqUlong ymin__ = MAX((ymin - originY()), 0);
	TqUlong xmaxplus1__ = MIN((xmaxplus1 - originX()), imageWidth());
	TqUlong ymaxplus1__ = MIN((ymaxplus1 - originY()), imageWidth());
	TqUlong bucketlinelen = elementSize * (xmaxplus1 - xmin); 
	TqUlong realLineLen = m_elementSize * imageWidth();
	
	boost::mutex::scoped_lock lock(mutex());

	const unsigned char* pdatarow = bucketData;
	// Calculate where in the bucket we are starting from if the window is cropped.
	TqInt row = 0;
	if(originY() > static_cast<TqUlong>(ymin))
		row = originY() - ymin;
	TqInt col = 0;
	if(originX() > static_cast<TqUlong>(xmin))
		col = originX() - xmin;
	pdatarow += (row * bucketlinelen) + (col * elementSize);

	if( data() && xmin__ >= 0 && ymin__ >= 0 && xmaxplus1__ <= imageWidth() && ymaxplus1__ <= imageHeight() )
	{
		TqUlong y;
		unsigned char *unrolled = m_data;
		unsigned char *realData = m_realData;

		for ( y = ymin__; y < ymaxplus1__; y++ )
		{
			TqUlong x;
			const unsigned char* _pdatarow = pdatarow;
			for ( x = xmin__; x < xmaxplus1__; x++ )
			{
				TqInt displayOffset = numChannels() * (( y * imageWidth() ) +  x );
				TqInt storageOffset = (( y * realLineLen ) + ( x * m_elementSize ) );
				TqUchar alpha = 255;
				/// \todo: Work out how to read alpha from the bucket data, taking into account sizes.
				std::vector<std::pair<std::string, TqInt> >::iterator channel;
				for(channel = m_channels.begin(); channel != m_channels.end(); ++channel)
				{
					switch(channel->second)
					{
						case PkDspyUnsigned16:
						{
							const TqUshort *svalue = reinterpret_cast<const TqUshort *>(_pdatarow);
							reinterpret_cast<TqUshort*>(&realData[storageOffset])[0] = svalue[0];
							CompositeAlpha((TqInt) (svalue[0]>>8), unrolled[displayOffset], alpha);
							_pdatarow += 2;
						}
						break;
						case PkDspySigned16:
						{
							const TqShort *svalue = reinterpret_cast<const TqShort *>(_pdatarow);
							reinterpret_cast<TqShort*>(&realData[storageOffset])[0] = svalue[0];
							CompositeAlpha((TqInt) (svalue[0]>>7), unrolled[displayOffset], alpha);
							_pdatarow += 2;
						}
						break;
						case PkDspyUnsigned32:
						{

							const TqUlong *lvalue = reinterpret_cast<const TqUlong *>(_pdatarow);
							reinterpret_cast<TqUlong*>(&realData[storageOffset])[0] = lvalue[0];
							CompositeAlpha((TqInt) (lvalue[0]>>24), unrolled[displayOffset], alpha);
							_pdatarow += 4;
						}
						break;
						case PkDspySigned32:
						{

							const TqLong *lvalue = reinterpret_cast<const TqLong *>(_pdatarow);
							reinterpret_cast<TqLong*>(&realData[storageOffset])[0] = lvalue[0];
							CompositeAlpha((TqInt) (lvalue[0]>>23), unrolled[displayOffset], alpha);
							_pdatarow += 4;
						}
						break;

						case PkDspyFloat32:
						{
							const TqFloat *fvalue = reinterpret_cast<const TqFloat *>(_pdatarow);
							reinterpret_cast<TqFloat*>(&realData[storageOffset])[0] = fvalue[0];
							CompositeAlpha((TqInt) (fvalue[0]*255.0), unrolled[displayOffset], alpha);
							_pdatarow += 4;
						}
						break;

						case PkDspySigned8:
						case PkDspyUnsigned8:
						default:
						{
							const TqUchar *cvalue = reinterpret_cast<const TqUchar *>(_pdatarow);
							reinterpret_cast<TqUchar*>(&realData[storageOffset])[0] = cvalue[0];
							CompositeAlpha((TqInt) cvalue[0], unrolled[displayOffset], alpha);
							_pdatarow += 1;
						}
						break;
					}
					++displayOffset;
					++storageOffset;
				}
			}
			pdatarow += bucketlinelen;
		}
		if(m_updateCallback)
			m_updateCallback(xmin__, ymin__, xmaxplus1__-xmin__, ymaxplus1__-ymin__);
	}
}

void CqDisplayServerImage::serialise(const std::string& folder)
{
	// Generate a unique name for the managed image in the specified folder.
	std::string _ext = CqFile::extension(name());
	std::string _basename = CqFile::baseName(name());
	std::stringstream strFilename;
	strFilename << folder << CqFile::pathSep() << name();
	FILE* tfile;
	int index = 1;
	while((tfile = fopen(strFilename.str().c_str(), "r")) != NULL)
	{
		fclose(tfile);
		strFilename.clear();
		strFilename.str("");
		strFilename << folder << CqFile::pathSep() << _basename<< "." << index << _ext;
		index++;
	}
		
	setFilename(strFilename.str());
	saveToTiff(strFilename.str());
}


void CqDisplayServerImage::saveToTiff(const std::string& filename)
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
		TqInt maxType = PkDspyUnsigned8;
		for(TqInt ichan = 0; ichan < numChannels(); ++ichan)
			maxType = std::max(maxType, channelType(ichan));

		// Write out an 8 bits per pixel integer image.
		if ( maxType == PkDspyUnsigned8 || maxType == PkDspySigned8 )
		{
			TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT );
			TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 8 );
		}
		else if(maxType == PkDspyFloat32)
		{
			/* use uncompressed IEEEFP pixels */
			TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP );
			TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 32 );
		}
		else if(maxType == PkDspySigned16 || maxType == PkDspyUnsigned16)
		{
			TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT );
			TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 16 );
		}
		else if(maxType == PkDspySigned32 || maxType == PkDspyUnsigned32)
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

// Define a base64 encoding stream iterator using the boost archive data flow iterators.
typedef 
    boost::archive::iterators::insert_linebreaks<         // insert line breaks every 72 characters
        boost::archive::iterators::base64_from_binary<    // convert binary values ot base64 characters
            boost::archive::iterators::transform_width<   // retrieve 6 bit integers from a sequence of 8 bit bytes
                const char *,
                6,
                8
            >
        > 
        ,72
    > 
    base64_text; // compose all the above operations in to a new iterator

TiXmlElement* CqDisplayServerImage::serialiseToXML()
{
	TiXmlElement* imageXML = new TiXmlElement("Image");

	TiXmlElement* typeXML = new TiXmlElement("Type");
	TiXmlText* typeText = new TiXmlText("managed");
	typeXML->LinkEndChild(typeText);
	imageXML->LinkEndChild(typeXML);

	TiXmlElement* nameXML = new TiXmlElement("Name");
	TiXmlText* nameText = new TiXmlText(name());
	nameXML->LinkEndChild(nameText);
	imageXML->LinkEndChild(nameXML);

	if(filename().empty())
	{
		TiXmlElement* dataXML = new TiXmlElement("Bitmap");
		std::stringstream base64Data;
		size_t dataLen = m_imageWidth * m_imageHeight * numChannels() * sizeof(TqUchar);
		std::copy(	base64_text(BOOST_MAKE_PFTO_WRAPPER(m_data)), 
					base64_text(BOOST_MAKE_PFTO_WRAPPER(m_data + dataLen)), 
					std::ostream_iterator<char>(base64Data));
		TiXmlText* dataTextXML = new TiXmlText(base64Data.str());
		dataTextXML->SetCDATA(true);
		dataXML->LinkEndChild(dataTextXML);
		imageXML->LinkEndChild(dataXML);
	}
	else
	{
		TiXmlElement* filenameXML = new TiXmlElement("Filename");
		TiXmlText* filenameText = new TiXmlText(filename());
		filenameXML->LinkEndChild(filenameText);
		imageXML->LinkEndChild(filenameXML);
	}

	return(imageXML);
}

void CqDisplayServerImage::reorderChannels()
{
	// If there are "r", "g", "b" and "a" channels, ensure they
	// are in the expected order.
	const char* elements[] = { "r", "g", "b", "a" };
	TqInt numElements = sizeof(elements) / sizeof(elements[0]);
	for(int elementIndex = 0; elementIndex < numElements; ++elementIndex)
	{
		for(std::vector<std::pair<std::string, TqInt> >::iterator channel = m_channels.begin(); channel != m_channels.end(); ++channel)
		{
			// If this entry in the channel list matches one in the expected list, 
			// move it to the right point in the channel list.
			if(channel->first.compare(elements[elementIndex]) == 0)
			{
				std::pair<std::string, TqInt> temp = m_channels[elementIndex];
				m_channels[elementIndex] = *channel;
				*channel = temp;
				break;
			}
		}
	}
}

END_NAMESPACE( Aqsis )
