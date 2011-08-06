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

// Checker pattern for background
inline float background(int x, int y)
{
	const int w = 16; // tile width
	int whichTile = (x/w + y/w) % 2;
	return 0.5f*(1 + whichTile);
}


CqImage::~CqImage()
{
}

void CqImage::initialize(int imageWidth, int imageHeight, int xorigin, int yorigin,
						 int frameWidth, int frameHeight, float clipNear,
						 float clipFar, const CqChannelList& channelList)
{
	boost::mutex::scoped_lock lock(mutex());

	m_imageWidth = imageWidth;
	m_imageHeight = imageHeight;
	m_originX = xorigin;
	m_originY = yorigin;
	m_frameWidth = frameWidth;
	m_frameHeight = frameHeight;
	m_clippingNear = clipNear;
	m_clippingFar = clipFar;

	if(channelList.numChannels() == 0)
		AQSIS_THROW_XQERROR(XqInternal, EqE_MissingData,
			"Not enough image channels to display");

	// Set up buffer for holding the full-precision data
	m_realData = boost::shared_ptr<CqMixedImageBuffer>(
			new CqMixedImageBuffer(channelList, m_imageWidth, m_imageHeight));

	fixupDisplayMap(channelList);
	// Set up 8-bit per pixel display image buffer
	m_displayData = QImage(m_imageWidth, m_imageHeight,
						   QImage::Format_ARGB32_Premultiplied);
	// Initialize to transparent
	m_displayData.fill(0);

	emit resized();
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
	setFilename(fileName);
	// \todo: Should read the origin and frame size out of the image.

	const CqTexFileHeader& header = texFile->header();
	TqUint width = header.width();
	TqUint height = header.height();
	m_imageWidth = width;
	m_imageHeight = height;
	// set size within larger cropped window
	const SqImageRegion displayWindow = header.find<Attr::DisplayWindow>(
			SqImageRegion(width, height, 0, 0) );
	m_frameWidth = displayWindow.width;
	m_frameHeight = displayWindow.height;
	m_originX = displayWindow.topLeftX;
	m_originY = displayWindow.topLeftY;
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
	m_displayData = QImage(width, height, QImage::Format_RGB32);
	updateDisplayData(*m_realData, 0, 0);
	// Compute the effective clipping range for z-buffers
	updateClippingRange();

	emit resized();
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


static void getCopyRegionSize(TqInt offset, TqInt srcWidth, TqInt destWidth,
							  TqInt& srcOffset, TqInt& destOffset,
							  TqInt& copyWidth)
{
	destOffset = max(offset, 0);
	srcOffset = max(-offset, 0);
	copyWidth = min(destWidth, offset+srcWidth) - destOffset;
}

void CqImage::updateDisplayData(const CqMixedImageBuffer& srcData,
								int x, int y)
{
	int destTopLeftX = 0;
	int srcTopLeftX = 0;
	int copyWidth = 0;
	getCopyRegionSize(x, srcData.width(), m_displayData.width(),
					  srcTopLeftX, destTopLeftX, copyWidth);
	int destTopLeftY = 0;
	int srcTopLeftY = 0;
	int copyHeight = 0;
	getCopyRegionSize(y, srcData.height(), m_displayData.height(),
					  srcTopLeftY, destTopLeftY, copyHeight);
	boost::shared_ptr<const CqImageChannel> rChan =
		srcData.channel(m_displayMap["r"], srcTopLeftX, srcTopLeftY,
						copyWidth, copyHeight);
	boost::shared_ptr<const CqImageChannel> gChan =
		srcData.channel(m_displayMap["g"], srcTopLeftX, srcTopLeftY,
						copyWidth, copyHeight);
	boost::shared_ptr<const CqImageChannel> bChan =
		srcData.channel(m_displayMap["b"], srcTopLeftX, srcTopLeftY,
						copyWidth, copyHeight);
	boost::shared_ptr<const CqImageChannel> aChan;
	if(srcData.channelList().hasChannel("a"))
		aChan = srcData.channel("a", srcTopLeftX, srcTopLeftY,
								copyWidth, copyHeight);
	if(isZBuffer())
	{
		const float invRange = 1/(m_clippingFar - m_clippingNear);
		const float* src = reinterpret_cast<const float*>(srcData.rawData());
		for(int j = 0; j < copyHeight; ++j)
		{
			QRgb* dest = destTopLeftX + reinterpret_cast<QRgb*>(
									m_displayData.scanLine(destTopLeftY + j));
			const float* row = src + srcTopLeftX +
							   srcData.width()*(srcTopLeftY + j);
			for(int i = 0, iend = copyWidth; i < iend; ++i)
			{
				TqUint8 val = static_cast<TqUint8>(
							clamp(255*(1-invRange*(row[i] - m_clippingNear)),
								  0.0f, 255.0f) );
				dest[i] = qRgb(val, val, val);
			}
		}
	}
	else
	{
		for(int j = 0; j < copyHeight; ++j)
		{
			QRgb* dest = destTopLeftX + reinterpret_cast<QRgb*>(
					               m_displayData.scanLine(destTopLeftY + j));
			const float* r = rChan->getRow(j);
			const float* g = gChan->getRow(j);
			const float* b = bChan->getRow(j);
			if(aChan)
			{
				const float* a = aChan->getRow(j);
				for(int i = 0; i < copyWidth; ++i)
				{
					// Get background color, weighted with alpha
					float c = (1 - a[i]) * background(
											destTopLeftX + m_originX + i,
											destTopLeftY + m_originY + j);
					dest[i] = qRgb(
							(TqUint8)clamp(255*(r[i] + c), 0.0f, 255.0f),
							(TqUint8)clamp(255*(g[i] + c), 0.0f, 255.0f),
							(TqUint8)clamp(255*(b[i] + c), 0.0f, 255.0f));
				}
			}
			else
			{
				for(int i = 0; i < copyWidth; ++i)
				{
					dest[i] = qRgb((TqUint8)clamp(255*r[i], 0.0f, 255.0f),
								   (TqUint8)clamp(255*g[i], 0.0f, 255.0f),
								   (TqUint8)clamp(255*b[i], 0.0f, 255.0f));
				}
			}
		}
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

	updateDisplayData(*m_realData, 0, 0);
	emit updated(m_originX, m_originY, m_imageWidth, m_imageHeight);
}

} // namespace Aqsis
