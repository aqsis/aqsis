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
#include <float.h>

#include <QTimer>

#include        <boost/archive/iterators/binary_from_base64.hpp>
#include	<boost/archive/iterators/base64_from_binary.hpp>
#include	<boost/archive/iterators/transform_width.hpp>
#include	<boost/archive/iterators/insert_linebreaks.hpp>
#include        <boost/archive/iterators/remove_whitespace.hpp>
#include	<boost/format.hpp>
#include	<boost/filesystem.hpp>

#include	"displayserverimage.h"

#include	<aqsis/math/math.h>
#include 	<aqsis/util/file.h>
#include 	<aqsis/util/logging.h>
#include	<aqsis/util/smartptr.h>

namespace Aqsis {

namespace {
struct NameMaps
{
    std::map<std::string, TqInt> nameToType;
    std::map<TqInt, std::string> typeToName;

    NameMaps()
    {
// Macros to initialise the type/name name/type maps.
#	define	INIT_TYPE_NAME_MAPS(name) \
	nameToType[#name] = name; \
	typeToName[name] = #name;
	// Fill in the typenames maps
	INIT_TYPE_NAME_MAPS(PkDspyFloat32);
	INIT_TYPE_NAME_MAPS(PkDspyUnsigned32);
	INIT_TYPE_NAME_MAPS(PkDspySigned32);
	INIT_TYPE_NAME_MAPS(PkDspyUnsigned16);
	INIT_TYPE_NAME_MAPS(PkDspySigned16);
	INIT_TYPE_NAME_MAPS(PkDspyUnsigned8);
	INIT_TYPE_NAME_MAPS(PkDspySigned8);
	INIT_TYPE_NAME_MAPS(PkDspyString);
	INIT_TYPE_NAME_MAPS(PkDspyMatrix);
#	undef INIT_TYPE_NAME_MAPS

    }
};
}

static NameMaps g_maps;

typedef
    boost::archive::iterators::transform_width<   // retrieve 6 bit integers from a sequence of 8 bit bytes
        boost::archive::iterators::binary_from_base64<    // convert binary values ot base64 characters
            boost::archive::iterators::remove_whitespace<
                std::string::const_iterator
            >
        >,
        8,
        6
    >
base64_binary; // compose all the above operations in to a new iterator

void CqDisplayServerImage::setSocket(QTcpSocket* socket)
{
  this->socket = socket;
  connect(socket, SIGNAL(readyRead()), this, SLOT(processMessage()));
}

//---------------------------------------------------------------------
/** Close the socket this client is associated with.
 */
void CqDisplayServerImage::close()
{
	// Recompute the effective near and far clipping
	updateClippingRange();
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

    void CqDisplayServerImage::processMessage()
    {
        QByteArray msg;
    
        if (socket->state() != QAbstractSocket::ConnectedState)
            return;

        const int bytesAvailable = socket->bytesAvailable();

        int bytesRead = 1;
        char c;
        socket->getChar(&c);
        while (c != '\0') {
            msg.append(c);
            if (!socket->getChar(&c))
	        break;
            bytesRead++;
        }

        if (bytesAvailable > bytesRead) // grab another chunk in half-a-second
            QTimer::singleShot(200, this, SLOT(processMessage()));

            // Parse the XML message sent.
            TiXmlDocument xmlMsg;
            xmlMsg.Parse(msg.data());
            // Get the root element, which is the base type of the message.
            TiXmlElement* root = xmlMsg.RootElement();

            if(root)
            {
                // Process the message based on its type.
                if(root->ValueStr().compare("Open") == 0)
                {
                    // Extract image dimensions, etc.
                    int xres = 640, yres = 480;
                    int xorigin = 0, yorigin = 0;
                    int xFrameSize = 0, yFrameSize = 0;
                    double clipNear = 0, clipFar = FLT_MAX;
                    const char* fname = "ri.pic";
                    CqChannelList channelList;

                    TiXmlElement* child = root->FirstChildElement("Dimensions");
                    if(child)
                    {
                        child->Attribute("width", &xres);
                        child->Attribute("height", &yres);
                    }

                    child = root->FirstChildElement("Name");
                    if(child)
                    {
                        const char* name = child->GetText();
                        if (name != NULL)
                            fname = name;
                    }
                    // Process the parameters
                    child = root->FirstChildElement("Parameters");
                    if(child)
                    {
                        TiXmlElement* param = child->FirstChildElement("IntsParameter");
                        while(param)
                        {
                            const char* name = param->Attribute("name");
                            if(std::string("origin").compare(name) == 0)
                            {
                                TiXmlElement* values = param->FirstChildElement("Values");
                                if(values)
                                {
                                    TiXmlElement* value = values->FirstChildElement("Int");
                                    value->Attribute("value", &xorigin);
                                    value = value->NextSiblingElement("Int");
                                    value->Attribute("value", &yorigin);
                                }
                            }
                            else if(std::string("OriginalSize").compare(name) == 0)
                            {
                                TiXmlElement* values = param->FirstChildElement("Values");
                                if(values)
                                {
                                    TiXmlElement* value = values->FirstChildElement("Int");
                                    value->Attribute("value", &xFrameSize);
                                    value = value->NextSiblingElement("Int");
                                    value->Attribute("value", &yFrameSize);
                                }
                            }
                            param = param->NextSiblingElement("IntsParameter");
                        }
                        param = child->FirstChildElement("FloatsParameter");
                        while(param)
                        {
                            const char* name = param->Attribute("name");
                            if(name == std::string("near"))
                            {
                                TiXmlElement* value = param->FirstChildElement("Values")
                                    ->FirstChildElement("Float");
                                value->Attribute("value", &clipNear);
                            }
                            else if(name == std::string("far"))
                            {
                                TiXmlElement* value = param->FirstChildElement("Values")
                                    ->FirstChildElement("Float");
                                value->Attribute("value", &clipFar);
                            }
                            param = param->NextSiblingElement("FloatsParameter");
                        }
                    }
                    child = root->FirstChildElement("Formats");
                    if(child)
                    {
                        TiXmlElement* format = child->FirstChildElement("Format");
                        while(format)
                        {
                            // Read the format type from the node.
                            const char* typeName = format->GetText();
                            const char* formatName = format->Attribute("name");
                            TqInt typeID = PkDspyUnsigned8;
                            std::map<std::string, TqInt>::iterator type;
                            if((type = g_maps.nameToType.find(typeName)) != g_maps.nameToType.end())
                                typeID = type->second;
                            channelList.addChannel(SqChannelInfo(formatName, chanFormatFromPkDspy(typeID)));

                            format = format->NextSiblingElement("Format");
                        }
                        // Ensure that the formats are in the right order.
                        channelList.reorderChannels();
                        // Send the reorganised formats back.
                        TiXmlDocument doc("formats.xml");
                        TiXmlDeclaration* decl = new TiXmlDeclaration("1.0","","yes");
                        TiXmlElement* formatsXML = new TiXmlElement("Formats");
                        for(CqChannelList::const_iterator ichan = channelList.begin();
                                ichan != channelList.end(); ++ichan)
                        {
                            TiXmlElement* formatv = new TiXmlElement("Format");
                            formatv->SetAttribute("name", ichan->name);
                            TiXmlText* formatText = new TiXmlText(g_maps.typeToName[pkDspyFromChanFormat(ichan->type)]);
                            formatv->LinkEndChild(formatText);
                            formatsXML->LinkEndChild(formatv);
                        }
                        doc.LinkEndChild(decl);
                        doc.LinkEndChild(formatsXML);

						std::stringstream message;
						message << doc << "\0";

						socket->write(message.str().c_str(), strlen(message.str().c_str()) + 1);
						socket->waitForBytesWritten();
                    }
                    setName(fname);
                    initialize(xres, yres, xorigin, yorigin, xFrameSize, yFrameSize,
							   clipNear, clipFar, channelList);
                }
                else if(root->ValueStr().compare("Data") == 0)
                {
                    TiXmlElement* dimensionsXML = root->FirstChildElement("Dimensions");
                    if(dimensionsXML)
                    {
                        int xmin, ymin, xmaxplus1, ymaxplus1, elementSize;
                        dimensionsXML->Attribute("xmin", &xmin);
                        dimensionsXML->Attribute("ymin", &ymin);
                        dimensionsXML->Attribute("xmaxplus1", &xmaxplus1);
                        dimensionsXML->Attribute("ymaxplus1", &ymaxplus1);
                        dimensionsXML->Attribute("elementsize", &elementSize);

                        TiXmlElement* bucketDataXML = root->FirstChildElement("BucketData");
                        if(bucketDataXML)
                        {
                            TiXmlText* dataText = static_cast<TiXmlText*>(bucketDataXML->FirstChild());
                            if(dataText)
                            {
                                int bucketlinelen = elementSize * (xmaxplus1 - xmin);
                                int count = bucketlinelen * (ymaxplus1 - ymin);
                                std::string data = dataText->Value();
                                std::vector<unsigned char> binaryData;
                                binaryData.reserve(count);
                                base64_binary ti_begin = base64_binary(data.begin());
                                std::size_t padding = 2 - count % 3;
                                while(--count > 0)
                                {
                                    binaryData.push_back(static_cast<char>(*ti_begin));
                                    ++ti_begin;
                                }
                                binaryData.push_back(static_cast<char>(*ti_begin));
                                if(padding > 1)
                                    ++ti_begin;
                                if(padding > 2)
                                    ++ti_begin;
                                acceptData(xmin, xmaxplus1, ymin, ymaxplus1, elementSize, &binaryData[0]);
                            }
                        }
                    }
                }
                else if(root->ValueStr().compare("Close") == 0)
                {
                    // Send and acknowledge.
                    TiXmlDocument doc("ack.xml");
                    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0","","yes");
                    TiXmlElement* formatsXML = new TiXmlElement("Acknowledge");
                    doc.LinkEndChild(decl);
                    doc.LinkEndChild(formatsXML);

		    std::stringstream message;
		    message << doc;

		    socket->write(message.str().c_str(), strlen(message.str().c_str()) + 1);
		    socket->waitForBytesWritten();
                    close();
                }
            }
    }



} // namespace Aqsis
