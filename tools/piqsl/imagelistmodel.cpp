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

#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtGui/QMessageBox>

namespace Aqsis {


ImageListModel::ImageListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}


void ImageListModel::loadFiles(const QStringList& fileNames)
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


}
