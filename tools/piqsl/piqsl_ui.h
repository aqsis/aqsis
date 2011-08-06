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

#ifndef AQSIS_PIQSL_UI_H_INCLUDED
#define AQSIS_PIQSL_UI_H_INCLUDED

#include <QtGui/QMainWindow>
#include <QtGui/QWidget>
#include <QtGui/QListView>

#include <QtGui/QStyledItemDelegate>

#include <boost/shared_ptr.hpp>

#include "image.h"

class QItemSelectionModel;
class QItemSelection;

namespace Aqsis {

class ImageListModel;
class PiqslListView;

/// Main window of the piqsl interface
class PiqslMainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        PiqslMainWindow(const QString& socketInterface, int socketPort,
                        const QStringList& filesToOpen);

    protected:
        void keyReleaseEvent(QKeyEvent* event);

        QSize sizeHint() const;

    private slots:
        /// Open file dialog so user can add images to the list
        void addImages();
        /// Remove currently selected images from the list
        void removeImage();
        /// Open an image library file
        void openLibrary();
        /// Save an image library file
        void saveLibrary();
        /// Open an image library file
        void saveLibraryAs();
        /// Display piqsl "about dialog"
        void aboutDialog();

    private:
        QString m_currentDirectory;
        ImageListModel* m_imageList;
        PiqslListView* m_imageListView;
        QString m_currentLibraryName;
};


/// Image viewer widget supporting zooming and panning
class PiqslImageView : public QWidget
{
    Q_OBJECT

    public:
        PiqslImageView(QWidget* parent = 0);

        void setSelectionModel(QItemSelectionModel* selectionModel);

    protected:
        void keyPressEvent(QKeyEvent* event);

        void wheelEvent(QWheelEvent* event);
        void mousePressEvent(QMouseEvent* event);
        void mouseMoveEvent(QMouseEvent* event);

        void resizeEvent(QResizeEvent* event);
        void paintEvent(QPaintEvent* event);

    private slots:
        void changeSelectedImage(const QItemSelection& selected,
                                 const QItemSelection& deselected);
        void centerImage();

        void imageUpdated(int x, int y, int w, int h);
        void imageResized();

    private:
        void setSelectedImage(const QModelIndexList& indexes);

        const ImageListModel* m_images;

        boost::shared_ptr<CqImage> m_image;  ///< current image
        float m_zoom;     ///< Amount of zoom
        QPointF m_tlPos;  ///< coordinates of image top left
        QPoint m_lastPos; ///< Last position in mouse drag
};


/// Custom list view which automatically selects any items added to the end.
///
/// In other respects, this is just like QListView.
class PiqslListView : public QListView
{
    Q_OBJECT

    public:
        PiqslListView(QWidget* parent = 0);

    protected slots:
        // reimplemented from QListView
        void rowsInserted(const QModelIndex& parent, int start, int end);
};


/// Handler for displaying CqImages in standard views like QListView
///
/// This is a delegate class for use with Qt's model-view framework,
/// customized for the display of CqImage items.
class ImageListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        ImageListDelegate(QObject* parent = 0);

        void paint(QPainter* painter, const QStyleOptionViewItem& option,
                   const QModelIndex& index) const;
};


} // namespace Aqsis

#endif // AQSIS_PIQSL_UI_H_INCLUDED
