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

#include "imagelistmodel.h"

#include <float.h>

#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtCore/QSocketNotifier>
#include <QtGui/QMessageBox>

#include <boost/thread/mutex.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION < 103700
#   include <boost/pfto.hpp>
#else
#   include <boost/serialization/pfto.hpp>
#endif


#include "displayserverimage.h"


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

boost::mutex g_XMLMutex;

class SocketDataHandler
{
    public:
        SocketDataHandler(boost::shared_ptr<CqDisplayServerImage> thisClient) : m_client(thisClient), m_done(false)
        {}

        void operator()()
        {
            handleData();
        }

        void handleData()
        {
            std::stringstream buffer;
            int count;

            // Read a message
            while(!m_done)
            {
                count = m_client->socket().recvData(buffer);
                if(count <= 0)
                    break;
                // Readbuf should now contain a complete message
                processMessage(buffer);
                buffer.str("");
                buffer.clear();
            }
        }

        int sendXMLMessage(TiXmlDocument& msg)
        {
            std::stringstream message;
            message << msg;

            return( m_client->socket().sendData( message.str() ) );
        }

        void processMessage(std::stringstream& msg)
        {
            boost::mutex::scoped_lock lock(g_XMLMutex);
            // Parse the XML message sent.
            TiXmlDocument xmlMsg;
            xmlMsg.Parse(msg.str().c_str());
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
                        sendXMLMessage(doc);
                    }
                    m_client->setName(fname);
                    m_client->initialize(xres, yres, xorigin, yorigin, xFrameSize, yFrameSize,
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
                                base64_binary ti_begin = base64_binary(BOOST_MAKE_PFTO_WRAPPER(data.begin()));
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
                                m_client->acceptData(xmin, xmaxplus1, ymin, ymaxplus1, elementSize, &binaryData[0]);
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
                    sendXMLMessage(doc);
                    m_client->close();
                    m_done = true;
                }
            }
        }

    private:
        boost::shared_ptr<CqDisplayServerImage> m_client;
        bool    m_done;
};



//------------------------------------------------------------------------------

ImageListModel::ImageListModel(QObject* parent,
                               const QString& socketInterface,
                               int socketPort)
    : QAbstractListModel(parent),
    m_images(),
    m_socket(),
    m_socketNotifier(0)
{
    if(m_socket.prepare(socketInterface.toStdString(), socketPort))
    {
        m_socketNotifier = new QSocketNotifier(m_socket,
                                               QSocketNotifier::Read, this);
        connect(m_socketNotifier, SIGNAL(activated(int)),
                this, SLOT(handleSocketData(int)));
    }
}


void ImageListModel::loadImageFiles(const QStringList& fileNames)
{
    beginInsertRows(QModelIndex(), m_images.size(),
                    m_images.size() + fileNames.size() - 1);
    for(int i = 0; i < fileNames.size(); ++i)
    {
        QString filePath = fileNames[i];
        try
        {
            boost::shared_ptr<CqImage> newImg(new CqImage());
            newImg->loadFromFile(filePath.toStdString().c_str());
            QFileInfo info(filePath);
            newImg->setName(info.fileName().toStdString().c_str());
            m_images.push_back(newImg);
        }
        catch(XqInternal& e)
        {
            QMessageBox::critical(0, tr("Error"),
                                  tr("Could not open file %1").arg(filePath));
        }
    }
    endInsertRows();
}


bool ImageListModel::saveImageLibrary(const QString& fileName) const
{
    std::string name = fileName.toStdString();
    boost::filesystem::path saveDir = boost::filesystem::path(name).branch_path();
    TiXmlDocument doc(name);
    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0","","yes");
    TiXmlElement* booksXML = new TiXmlElement("Books");

    TiXmlElement* bookXML = new TiXmlElement("Book");
    bookXML->SetAttribute("name", "Book1");
    booksXML->LinkEndChild(bookXML);
    TiXmlElement* imagesXML = new TiXmlElement("Images");

    for(int i = 0; i < (int)m_images.size(); ++i)
    {
        // Serialise the image first.
        m_images[i]->serialise(saveDir);
        TiXmlElement* imageXML = m_images[i]->serialiseToXML();
        imagesXML->LinkEndChild(imageXML);
    }
    bookXML->LinkEndChild(imagesXML);

    doc.LinkEndChild(decl);
    doc.LinkEndChild(booksXML);
    return doc.SaveFile(name);
}


bool ImageListModel::appendImageLibrary(const QString& fileName)
{
    // m_currentConfigName = name; FIXME
    TiXmlDocument doc(fileName.toStdString());
    if(doc.LoadFile())
    {
        TiXmlElement* booksXML = doc.RootElement();
        if(booksXML)
        {
            TiXmlElement* bookXML = booksXML->FirstChildElement("Book");
            while(bookXML)
            {
                //std::string bookName = bookXML->Attribute("name");
                TiXmlElement* imagesXML = bookXML->FirstChildElement("Images");
                if(imagesXML)
                {
                    TiXmlElement* imageXML = imagesXML->FirstChildElement("Image");
                    while(imageXML)
                    {
                        std::string imageName("");
                        std::string imageFilename("");
                        TiXmlElement* nameXML = imageXML->FirstChildElement("Name");
                        if(nameXML && nameXML->GetText())
                            imageName = nameXML->GetText();
                        TiXmlElement* fileNameXML = imageXML->FirstChildElement("Filename");
                        if(fileNameXML && fileNameXML->GetText())
                            imageFilename = fileNameXML->GetText();
                        boost::shared_ptr<CqImage> newImage(new CqImage(imageName));
                        newImage->loadFromFile(imageFilename);
                        beginInsertRows(QModelIndex(), m_images.size(),
                                        m_images.size());
                        m_images.push_back(newImage);
                        endInsertRows();
                        imageXML = imageXML->NextSiblingElement("Image");
                    }
                }
                bookXML = bookXML->NextSiblingElement("Book");
            }
        }
        return true;
    }
    else
    {
        QMessageBox::critical(0, tr("Error"),
                        tr("Failed to load image list %1").arg(fileName));
        return false;
    }
}


int ImageListModel::rowCount(const QModelIndex& parent) const
{
    return m_images.size();
}


QVariant ImageListModel::data(const QModelIndex & index, int role) const
{
    if(!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole))
        return QVariant();
    if(index.row() >= (int)m_images.size())
        return QVariant();

    return QVariant::fromValue(m_images[index.row()]);
}


// Abortive support for drag & drop.
#if 0
Qt::ItemFlags ImageListModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled;
}


bool ImageListModel::setData(const QModelIndex &index, const QVariant &value,
                             int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        m_images[index.row()] = value.value<boost::shared_ptr<CqImage> >();
        emit dataChanged(index, index);
        return true;
    }
    return false;
}
#endif


bool ImageListModel::removeRows(int position, int rows,
                                const QModelIndex &parent)
{
    if(position < 0 || position + rows > (int)m_images.size())
        return false;
    beginRemoveRows(QModelIndex(), position, position+rows-1);
    m_images.erase(m_images.begin()+position, m_images.begin()+position+rows);
    endRemoveRows();
    return true;
}


bool ImageListModel::insertRows(int position, int rows,
                                const QModelIndex &parent)
{
    if(position < 0 || position > (int)m_images.size())
        return false;
    beginInsertRows(QModelIndex(), position, position+rows-1);
    m_images.insert(m_images.begin() + position, rows,
                    boost::shared_ptr<CqImage>());
    endInsertRows();
    return true;
}


/// Activated when data is waiting to be read on the main socket
void ImageListModel::handleSocketData(int /*ignored*/)
{
    boost::shared_ptr<CqDisplayServerImage> newImage(new CqDisplayServerImage());
    newImage->setName("Unnamed");

    if(m_socket.accept(newImage->socket()))
    {
        m_socketReadThreads.push_back(boost::shared_ptr<boost::thread>(
                                new boost::thread(SocketDataHandler(newImage))));
        beginInsertRows(QModelIndex(), m_images.size(), m_images.size());
        m_images.push_back(newImage);
        endInsertRows();
        connect(newImage.get(), SIGNAL(updated(int,int,int,int)),
                this, SLOT(imageUpdated(int,int,int,int)));
    }
}


/// Activated when one of the internally held images changed
void ImageListModel::imageUpdated(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
    QObject* im = sender();
    // Search through images, to find the row index of the one which sent the
    // signal.
    for(int i = 0; i < (int)m_images.size(); ++i)
    {
        if(im == static_cast<QObject*>(m_images[i].get()))
        {
            QModelIndex index = createIndex(i, 0);
            emit dataChanged(index, index);
            return;
        }
    }
    assert(0 && "ImageListModel: image row not found");
}


}
