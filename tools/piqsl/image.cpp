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

#include <aqsis/aqsis.h>

#include <float.h>

#include <boost/format.hpp>

#include <aqsis/version.h>
#include <aqsis/util/logging.h>
#include <aqsis/ri/ndspy.h>
#include <aqsis/tex/io/itexinputfile.h>
#include <aqsis/tex/io/itexoutputfile.h>

#include "image.h"

namespace Aqsis {

CqImage::~CqImage()
{
}

void CqImage::prepareImageBuffers(const CqChannelList& channelList)
{
	boost::mutex::scoped_lock lock(mutex());

	if(channelList.numChannels() == 0)
		AQSIS_THROW_XQERROR(XqInternal, EqE_MissingData,
			"Not enough image channels to display");

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

	// Compute the effective clipping range for z-buffers
	updateClippingRange();

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
	header.setWidth(m_realData->width());
	header.setHeight(m_realData->height());
	header.channelList() = m_realData->channelList();
	// Informational strings
	header.set<Attr::Software>( (boost::format("Aqsis %s (%s %s)")
			 % AQSIS_VERSION_STR % __DATE__ % __TIME__).str());

	header.set<Attr::DisplayWindow>(SqImageRegion(m_frameWidth, m_frameHeight, m_originX, m_originY));
	header.set<Attr::PixelAspectRatio>(1.0);

	// Set some default compression scheme for now - later we can accept user
	// input for this.
	header.set<Attr::Compression>("lzw");

	// \todo: Attributes which might be good to add:
	//   Host computer
	//   Image description
	//   Transformation matrices

	try
	{
		// Now create the image, and output the pixel data.
		boost::shared_ptr<IqTexOutputFile> outFile
			= IqTexOutputFile::open(fileName, ImageFile_Tiff, header);

		// Write all pixels out at once.
		outFile->writePixels(*m_realData);
	}
	catch(XqInternal& e)
	{
		Aqsis::log() << error << "Could not save image \"" << fileName << "\": "
			<< e.what() << "\n";
		return;
	}
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

/** \brief Update the [near,far] clipping interval for z-buffer data
 *
 * If the data represents a z-buffer, iterate across and update the clipping
 * range reported by the image to reflect the minimum and maximum data present
 * in the image.  Occurances of FLT_MAX in the data are ignored since that
 * represents regions without visible objects.
 */
void CqImage::updateClippingRange()
{
	if(!isZBuffer())
		return;

	assert(m_realData);

	TqFloat maxD = 0;
	TqFloat minD = FLT_MAX;
	// Iterate through the map, updating the min and max depths.
	const TqFloat* buf = reinterpret_cast<const TqFloat*>(m_realData->rawData());
	for(int i = 0, size = m_realData->width()*m_realData->height(); i < size; ++i)
	{
		TqFloat z = buf[i];
		// Ignore any depths greater than 0.5*FLT_MAX, since they more than
		// likely come from regions where the max depth was averaged with a
		// surface near the camera, and will result in a non-useful
		// normalisation.
		if(z >= 0.5f*FLT_MAX)
			continue;
		if(z > maxD)
			maxD = z;
		else if(z < minD)
			minD = z;
	}
	if(minD == FLT_MAX)
		minD = 0;
	// Make sure a finite range is reported when no data is present.
	if(maxD <= minD)
		maxD = minD+1e-5;
	m_clippingNear = minD;
	m_clippingFar = maxD;

	if(m_updateCallback)
		m_updateCallback(-1, -1, -1, -1);
}

} // namespace Aqsis
