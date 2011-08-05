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
		\brief Implements an image class getting it's data from the Dspy server.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/


#include	<aqsis/aqsis.h>

#include	<map>
#include	<algorithm>

#include	<boost/archive/iterators/base64_from_binary.hpp>
#include	<boost/archive/iterators/transform_width.hpp>
#include	<boost/archive/iterators/insert_linebreaks.hpp>
#include	<boost/format.hpp>
#include	<boost/filesystem.hpp>

#include	"displayserverimage.h"

#include	<aqsis/math/math.h>
#include 	<aqsis/util/file.h>
#include 	<aqsis/util/logging.h>
#include	<aqsis/util/smartptr.h>

namespace Aqsis {

//---------------------------------------------------------------------
/** Close the socket this client is associated with.
 */
void CqDisplayServerImage::close()
{
	// Recompute the effective near and far clipping
	updateClippingRange();
	m_socket.close();
}


//----------------------------------------------------------------------

void CqDisplayServerImage::acceptData(TqUlong xmin, TqUlong xmaxplus1, TqUlong ymin, TqUlong ymaxplus1, TqInt elementSize, const unsigned char* bucketData )
{
	assert(elementSize == m_realData->channelList().bytesPerPixel());

	// yuck.  To fix the casts, refactor Image to use signed ints.
	TqInt cropXmin = static_cast<TqInt>(xmin) - static_cast<TqInt>(m_originX);
	TqInt cropYmin = static_cast<TqInt>(ymin) - static_cast<TqInt>(m_originY);

	boost::mutex::scoped_lock lock(mutex());

	if(m_realData && !m_displayData.isNull())
	{
		// The const_cast below is ugly, but I don't see how to avoid it
		// without some notion of "const constructor" which isn't present in
		// C++
		const CqMixedImageBuffer bucketBuf(m_realData->channelList(),
				boost::shared_array<TqUchar>(const_cast<TqUchar*>(bucketData),
					nullDeleter), xmaxplus1 - xmin, ymaxplus1 - ymin);

		m_realData->copyFrom(bucketBuf, cropXmin, cropYmin);
		updateDisplayData(bucketBuf, cropXmin, cropYmin);
		emit updated(xmin, ymin, xmaxplus1-xmin, ymaxplus1-ymin);
	}
}

void CqDisplayServerImage::serialise(const boost::filesystem::path& directory)
{
	namespace fs = boost::filesystem;
	fs::path fileName(name());
	// Generate a unique name for the managed image in the specified directory.
	std::string ext = fs::extension(fileName);
	std::string base = fs::basename(fileName);
	fs::path uniquePath = directory/fileName;
	TqInt index = 1;
	while(fs::exists(uniquePath))
	{
		uniquePath = directory/(boost::format("%s.%d%s") % base % index % ext).str();
		++index;
	}

	setFilename(native(uniquePath));
	saveToFile(native(uniquePath));
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

#if 0
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
		size_t dataLen = m_displayData.width() * m_displayData.height() * numChannels() * sizeof(TqUchar);
		std::copy(	base64_text(BOOST_MAKE_PFTO_WRAPPER(m_displayData->rawData())), 
					base64_text(BOOST_MAKE_PFTO_WRAPPER(m_displayData->rawData() + dataLen)), 
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
#endif

} // namespace Aqsis
