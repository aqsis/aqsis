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
		\brief Implements the basic image functionality.
		\author Paul C. Gregory (pgregory@aqsis.org)
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

void CqImage::prepareImageBuffers(const CqChannelInfoList& channelsInfo)
{
	boost::mutex::scoped_lock lock(mutex());

	if(channelsInfo.numChannels() == 0)
		throw XqInternal("Not enough image channels to display", __FILE__, __LINE__);

	// Set up buffer for holding the full-precision data
	m_realData = boost::shared_ptr<CqImageBuffer>(
			new CqImageBuffer(channelsInfo, m_imageWidth, m_imageHeight));

	fixupDisplayMap(channelsInfo);
	// Set up 8-bit per pixel display image buffer
	m_displayData = boost::shared_ptr<CqImageBuffer>(
			new CqImageBuffer(CqChannelInfoList::displayChannels(),
				m_imageWidth, m_imageHeight));
	m_displayData->initToCheckerboard();
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

// custom deallocation function for use with boost::shared_ptr<TIFF>.
void safeTiffClose(TIFF* tif)
{
	if(tif)
		TIFFClose(tif);
}

void CqImage::loadFromTiff(const std::string& filename)
{
	boost::shared_ptr<TIFF> tif(TIFFOpen(filename.c_str(), "r"), safeTiffClose);
	boost::mutex::scoped_lock lock(mutex());
	// Read image into a buffer, and check for success.
	m_realData = CqImageBuffer::loadFromTiff(tif.get());
	if(!m_realData)
	{
		// \todo: Should we do something else here as well?
		Aqsis::log() << Aqsis::error << "Could not load image \"" << filename << "\"\n";
		return;
	}
	// Reading succeeded.  Read in additional data from the tiff file & set
	// some variables accordingly.
	setFilename(filename);
	// \todo: Should read the origin and frame size out of the image.
	setOrigin(0,0);
	TqUint width = m_realData->width();
	TqUint height = m_realData->height();
	setImageSize(width, height);
	setFrameSize(width, height);
	Aqsis::log() << Aqsis::info << "Loaded image " << filename
		<< " [" << width << "x" << height << "x"
		<< m_realData->numChannels()
		<< "] (PkDspyType = " << m_realData->channelsInfo()[0].type << ")"
		<< std::endl;
	TqChar *description = "";
	if(TIFFGetField(tif.get(), TIFFTAG_IMAGEDESCRIPTION, &description) != 1)
		TIFFGetField(tif.get(), TIFFTAG_SOFTWARE, &description);
	setDescription(description);

	fixupDisplayMap(m_realData->channelsInfo());
	// Quantize and display the data
	m_displayData = boost::shared_ptr<CqImageBuffer>(
			new CqImageBuffer(CqChannelInfoList::displayChannels(), width, height));
	m_displayData->initToCheckerboard();
	m_displayData->compositeOver(*m_realData, m_displayMap);

	if(m_updateCallback)
		m_updateCallback(-1, -1, -1, -1);
}

void CqImage::saveToTiff(const std::string& filename) const
{
	boost::mutex::scoped_lock lock(mutex());
	boost::shared_ptr<TIFF> pOut(TIFFOpen(filename.c_str(), "r"), safeTiffClose);

	if(!pOut)
	{
		// \todo: Should we do something else here as well?
		Aqsis::log() << Aqsis::error << "Could not save image to file \"" << filename << "\"\n";
		return;
	}

	// Write software version information
	char version[ 80 ];
	sprintf( version, "%s %s (%s %s)", STRNAME, VERSION_STR, __DATE__, __TIME__);
	TIFFSetField( pOut.get(), TIFFTAG_SOFTWARE, ( char* ) version );
	// Compute the date & time and write to the tiff.
	time_t long_time;
	time( &long_time );           /* Get time as long integer. */
	struct tm* ct = localtime( &long_time ); /* Convert to local time. */
	int year=1900 + ct->tm_year;
	char datetime[21];
	sprintf(datetime, "%04d:%02d:%02d %02d:%02d:%02d", year, ct->tm_mon + 1,
			ct->tm_mday, ct->tm_hour, ct->tm_min, ct->tm_sec);
	TIFFSetField( pOut.get(), TIFFTAG_DATETIME, datetime);
	// Set position tags for dealing with cropped images.
	TIFFSetField( pOut.get(), TIFFTAG_XPOSITION, ( float ) originX() );
	TIFFSetField( pOut.get(), TIFFTAG_YPOSITION, ( float ) originY() );
	// Set x and y resolutions to some default values.
	TIFFSetField( pOut.get(), TIFFTAG_XRESOLUTION, (float) 1.0 );
	TIFFSetField( pOut.get(), TIFFTAG_YRESOLUTION, (float) 1.0 );
	// Now write the actual image data.
	m_realData->saveToTiff(pOut.get());

	// Old (obsolete??) stuff inherited from before factoring out much of the
	// tiff saving code into CqImageBuffer:

//		if (!image->m_hostname.empty())
//			TIFFSetField( pOut.get(), TIFFTAG_HOSTCOMPUTER, image->m_hostname.c_str() );
//	char mydescription[80];
//	TIFFSetField( pOut.get(), TIFFTAG_IMAGEDESCRIPTION, mydescription);
//	setDescription(std::string(mydescription));
	// Set compression type
//	TIFFSetField( pOut.get(), TIFFTAG_COMPRESSION, image->m_compression );
//	if ( image->m_compression == COMPRESSION_JPEG )
//		TIFFSetField( pOut.get(), TIFFTAG_JPEGQUALITY, image->m_quality );
//	TIFFSetField( pOut.get(), TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, image->m_matWorldToCamera );
//	TIFFSetField( pOut.get(), TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, image->m_matWorldToScreen );
}

void CqImage::fixupDisplayMap(const CqChannelInfoList& channelsInfo)
{
	// Validate the mapping between the display channels and the underlying
	// image channels.
	if(!channelsInfo.hasChannel("r"))
		m_displayMap["r"] = channelsInfo[0].name;
	else
		m_displayMap["r"] = "r";

	if(!channelsInfo.hasChannel("g"))
		m_displayMap["g"] = channelsInfo[0].name;
	else
		m_displayMap["g"] = "g";

	if(!channelsInfo.hasChannel("b"))
		m_displayMap["b"] = channelsInfo[0].name;
	else
		m_displayMap["b"] = "b";
}


END_NAMESPACE( Aqsis )
