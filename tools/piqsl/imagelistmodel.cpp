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
#include <filesystem>

#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtCore/QSocketNotifier>
#include <QtWidgets/QMessageBox>

#include <boost/thread/mutex.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include <boost/version.hpp>


#include "displayserverimage.h"


namespace Aqsis {

//------------------------------------------------------------------------------

ImageListModel::ImageListModel(QObject* parent,
                               const QString& socketInterface,
                               int socketPort)
    : QAbstractListModel(parent),
    m_images()
{
    if (m_server.listen(QHostAddress(socketInterface), socketPort))
    {
      connect(&m_server, SIGNAL(newConnection()), this, SLOT(handleConnection()));
    }
}

void ImageListModel::handleConnection()
{
    QTcpSocket *clientConnection = m_server.nextPendingConnection();
    QSharedPointer<CqDisplayServerImage> newImage(new CqDisplayServerImage());
    newImage->setName("Unnamed");
    newImage->setSocket(clientConnection);
  
    beginInsertRows(QModelIndex(), m_images.size(), m_images.size());
    m_images.append(newImage);
    endInsertRows();
    connect(newImage.data(), SIGNAL(updated(int,int,int,int)),
			this, SLOT(imageUpdated(int,int,int,int)));
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
            QSharedPointer<CqImage> newImg(new CqImage());
            newImg->loadFromFile(filePath.toStdString().c_str());
            QFileInfo info(filePath);
            newImg->setName(info.fileName().toStdString().c_str());
            m_images.append(newImg);
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
    std::filesystem::path saveDir = std::filesystem::path(name).parent_path();
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
        m_images[i]->serialise(saveDir.string());
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
                        QSharedPointer<CqImage> newImage(new CqImage(imageName));
                        newImage->loadFromFile(imageFilename);
                        beginInsertRows(QModelIndex(), m_images.size(),
                                        m_images.size());
                        m_images.append(newImage);
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
                    QSharedPointer<CqImage>());
    endInsertRows();
    return true;
}

/// Activated when one of the internally held images changed
void ImageListModel::imageUpdated(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
    QObject* im = sender();
    // Search through images, to find the row index of the one which sent the
    // signal.
    for(int i = 0; i < (int)m_images.size(); ++i)
    {
        if(im == static_cast<QObject*>(m_images[i].data()))
        {
            QModelIndex index = createIndex(i, 0);
            emit dataChanged(index, index);
            return;
        }
    }
    assert(0 && "ImageListModel: image row not found");
}

}
