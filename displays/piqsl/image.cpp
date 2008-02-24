// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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

#include <boost/format.hpp>

#include "version.h"
#include "image.h"
#include "framebuffer.h"
#include "logging.h"
#include "ndspy.h"
#include "itexinputfile.h"
#include "itexoutputfile.h"

namespace Aqsis {

CqImage::~CqImage()
{
}

void CqImage::setZoom(TqInt zoom)
{
	if(zoom <= 0)
		throw XqInternal("Negative or zero zoom specified.", __FILE__, __LINE__);

	if(zoom == m_zoom || !m_displayData)
		return;

	boost::mutex::scoped_lock lock(mutex());
	m_zoom = zoom;

	if(!m_zoomDisplayData)
	{
		m_zoomDisplayData.reset( new CqMixedImageBuffer(
				m_displayData->channelList(),
				m_displayData->width()*m_zoom,
				m_displayData->height()*m_zoom)
			);
	}

	// Make sure the zoom buffer has the right size
	m_zoomDisplayData->resize( m_displayData->width()*m_zoom,
			m_displayData->height()*m_zoom, m_displayData->channelList()
			);

	// Copy channels from display data to zoom data, zooming them as we go.
	for(TqInt chan = 0; chan < m_displayData->channelList().numChannels();
			++chan)
	{
		m_zoomDisplayData->channel(chan)->copyFrom(
				CqImageChannelZoom(*(m_displayData->channel(chan)), m_zoom) );
	}
	/// \todo: This is a nasty hack - the frame height isn't preserved
	//anywhere else, so this will be broken when we resize cropped images...
	//
	// The fix is time-consuming though; the piqsl CqImage and
	// CqDisplayServerImage classes really need a thorough refactor to better
	// seperate two distinct concerns:
	//
	// 1) Holding and updating the underlying data piped in from aqsis or a
	//    file
	// 2) Formatting that data for display.
	setFrameSize(m_zoomDisplayData->width(), m_zoomDisplayData->height());
	setImageSize(m_zoomDisplayData->width(), m_zoomDisplayData->height());

	if(m_updateCallback)
		m_updateCallback(-1, -1, -1, -1);
}

void CqImage::prepareImageBuffers(const CqChannelList& channelList)
{
	boost::mutex::scoped_lock lock(mutex());

	if(channelList.numChannels() == 0)
		throw XqInternal("Not enough image channels to display", __FILE__, __LINE__);

	// Set up buffer for holding the full-precision data
	m_realData = boost::shared_ptr<CqMixedImageBuffer>(
			new CqMixedImageBuffer(channelList, m_imageWidth, m_imageHeight));

	fixupDisplayMap(channelList);
	// Set up 8-bit per pixel display image buffer
	m_displayData = boost::shared_ptr<CqMixedImageBuffer>(
			new CqMixedImageBuffer(CqChannelList::displayChannels(),
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

void CqImage::loadFromFile(const std::string& fileName, TqInt imageIndex)
{
	boost::mutex::scoped_lock lock(mutex());

	boost::shared_ptr<IqTexInputFile> texFile;
	try
	{
		texFile = IqTexInputFile::open(fileName);
		if(imageIndex > 0)
		{
			IqMultiTexInputFile* multiFile = dynamic_cast<IqMultiTexInputFile*>(texFile.get());
			if(multiFile && imageIndex < multiFile->numSubImages())
			{
				multiFile->setImageIndex(imageIndex);
				m_imageIndex = imageIndex;
			}
			else
				return;
		}
		else
			m_imageIndex = 0;
	}
	catch(XqInternal& e)
	{
		Aqsis::log() << error << "Could not load image \"" << fileName << "\": "
			<< e.what() << "\n";
		return;
	}
	setFilename(fileName);
	// \todo: Should read the origin and frame size out of the image.

	const CqTexFileHeader& header = texFile->header();
	TqUint width = header.width();
	TqUint height = header.height();
	setImageSize(width, height);
	// set size within larger cropped window
	const SqImageRegion displayWindow = header.find<Attr::DisplayWindow>(
			SqImageRegion(width, height, 0, 0) );
	setFrameSize(displayWindow.width, displayWindow.height);
	setOrigin(displayWindow.topLeftX, displayWindow.topLeftY);
	// descriptive strings
	setDescription(header.find<Attr::Description>(
				header.find<Attr::Software>("No description") ).c_str());

	m_realData = boost::shared_ptr<CqMixedImageBuffer>(new CqMixedImageBuffer());
	texFile->readPixels(*m_realData);

	Aqsis::log() << Aqsis::info << "Loaded image " << fileName
		<< " [" << width << "x" << height << " : "
		<< texFile->header().channelList() << "]" << std::endl;

	fixupDisplayMap(m_realData->channelList());
	// Quantize and display the data
	m_displayData = boost::shared_ptr<CqMixedImageBuffer>(
			new CqMixedImageBuffer(CqChannelList::displayChannels(), width, height));
	m_displayData->initToCheckerboard();
	m_displayData->compositeOver(*m_realData, m_displayMap);

	if(m_updateCallback)
		m_updateCallback(-1, -1, -1, -1);
}

void CqImage::loadNextSubImage()
{
	loadFromFile(filename(), m_imageIndex+1);
}

void CqImage::loadPrevSubImage()
{
	if(m_imageIndex-1 >= 0)
		loadFromFile(filename(), m_imageIndex-1);
}

void CqImage::reloadFromFile()
{
	/// \todo Warning!  Probable bad behaviour for the case when the image
	// comes from aqsis rather than a file.
	loadFromFile(filename(), m_imageIndex);
}

void CqImage::saveToFile(const std::string& fileName) const
{
	boost::mutex::scoped_lock lock(mutex());

	CqTexFileHeader header;

	// Required attributes
	header.set<Attr::Width>(m_realData->width());
	header.set<Attr::Height>(m_realData->height());
	header.set<Attr::ChannelList>(m_realData->channelList());
	// Informational strings
	header.set<Attr::Software>( (boost::format("%s %s (%s %s)")
			 % STRNAME % VERSION_STR % __DATE__ % __TIME__).str());

	header.set<Attr::DisplayWindow>(SqImageRegion(m_frameWidth, m_frameHeight, m_originX, m_originY));
	header.set<Attr::PixelAspectRatio>(1.0);

	// Set some default compression scheme for now - later we can accept user
	// input for this.
	header.set<Attr::Compression>("lzw");

	// \todo: Attributes which might be good to add:
	//   Host computer
	//   Image description
	//   Transformation matrices

	// Now create the image, and output the pixel data.
	boost::shared_ptr<IqTexOutputFile> outFile
		= IqTexOutputFile::open(fileName, ImageFile_Tiff, header);

	// Write all pixels out at once.
	outFile->writePixels(*m_realData);
}

void CqImage::fixupDisplayMap(const CqChannelList& channelList)
{
	// Validate the mapping between the display channels and the underlying
	// image channels.
	if(!channelList.hasChannel("r"))
		m_displayMap["r"] = channelList[0].name;
	else
		m_displayMap["r"] = "r";

	if(!channelList.hasChannel("g"))
		m_displayMap["g"] = channelList[0].name;
	else
		m_displayMap["g"] = "g";

	if(!channelList.hasChannel("b"))
		m_displayMap["b"] = channelList[0].name;
	else
		m_displayMap["b"] = "b";
}


} // namespace Aqsis
