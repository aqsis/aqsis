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

#ifndef AQSIS_IMAGELISTMODEL_H_INCLUDED
#define AQSIS_IMAGELISTMODEL_H_INCLUDED

#include <QtCore/QAbstractListModel>

#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>

#include <aqsis/util/socket.h>
#include "image.h"

class QSocketNotifier;

namespace Aqsis {

/// Manage a list of images, and accept new images via socket.
///
/// This is a model class for use with Qt's model-view framework.
class ImageListModel : public QAbstractListModel
{
    Q_OBJECT

    public:
        ImageListModel(QObject* parent, const QString& socketInterface,
                       int socketPort);

        /// Load images from the provided list of file names
        void loadImageFiles(const QStringList& fileNames);

        /// Save the list of images to an XML file with the given file name.
        ///
        /// Returns true on success.
        bool saveImageLibrary(const QString& fileName) const;
        /// Load a list of images from an XML file with the given file name.
        ///
        /// Appends to the currently open image list.  Returns true on
        /// success.
        bool appendImageLibrary(const QString& fileName);

        // Overridden from QAbstractListModel
        int rowCount(const QModelIndex& parent = QModelIndex()) const;
        QVariant data(const QModelIndex & index, int role) const;

        bool removeRows(int position, int rows,
                        const QModelIndex &parent = QModelIndex());
        bool insertRows(int position, int rows,
                        const QModelIndex &parent = QModelIndex());

#if 0
        // Abortive support for drag & drop.
        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex &index, const QVariant &value,
                     int role = Qt::EditRole);
        Qt::DropActions supportedDropActions() const
        {
            return Qt::MoveAction;
        }
#endif

    private slots:
        void handleSocketData(int /*ignored*/);

        void imageUpdated(int x, int y, int w, int h);

    private:
        std::vector<boost::shared_ptr<CqImage> > m_images;
        CqSocket m_socket;
        std::vector<boost::shared_ptr<boost::thread> > m_socketReadThreads;
        QSocketNotifier* m_socketNotifier;
};

}

Q_DECLARE_METATYPE(boost::shared_ptr<Aqsis::CqImage>)


#endif // AQSIS_IMAGELISTMODEL_H_INCLUDED
