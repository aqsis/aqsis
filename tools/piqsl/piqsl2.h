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

#ifndef AQSIS_EQSL_H_INCLUDED
#define AQSIS_EQSL_H_INCLUDED

#include <QtGui/QMainWindow>
#include <QtGui/QWidget>

#include <QtCore/QAbstractListModel>
#include <QtGui/QStyledItemDelegate>

#include <boost/shared_ptr.hpp>

#include "image.h"

namespace Aqsis {

/// Main window of the piqsl interface
class PiqslMainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        PiqslMainWindow();

    protected:
        void keyReleaseEvent(QKeyEvent* event);

        QSize sizeHint() const;

    private slots:
        void aboutDialog();
};


/// Image viewer widget supporting zooming and panning
class PiqslImageView : public QWidget
{
    Q_OBJECT

    public:
        PiqslImageView(QWidget* parent = 0);

    protected:
        void keyPressEvent(QKeyEvent* event);

        void wheelEvent(QWheelEvent* event);
        void mousePressEvent(QMouseEvent* event);
        void mouseMoveEvent(QMouseEvent* event);

        void resizeEvent(QResizeEvent* event);
        void paintEvent(QPaintEvent* event);

    private:
        boost::shared_ptr<CqImage> m_image;  ///< current image
        int m_zoom;       ///< Amount of zoom
        int m_x;          ///< x coordinate of image top left
        int m_y;          ///< y coordinate of image top left
        QPoint m_lastPos; ///< Last position in mouse drag
};


/// Hold a list of images
///
/// This is a "model" class for use with Qt's model-view framework.
class ImageListModel : public QAbstractListModel
{
    Q_OBJECT

    public:
        ImageListModel(QObject* parent = 0);

        int rowCount(const QModelIndex& parent) const;

        QVariant data(const QModelIndex & index, int role) const;

    private:
        std::vector<boost::shared_ptr<CqImage> > m_images;
};


/// Handler for displaying CqImages in standard views like QListView
///
/// This is a "delegate" class for use with Qt's model-view framework.
class ImageListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        ImageListDelegate(QObject* parent = 0);

        void paint(QPainter* painter, const QStyleOptionViewItem& option,
                   const QModelIndex& index) const;
};


} // namespace Aqsis

Q_DECLARE_METATYPE(boost::shared_ptr<Aqsis::CqImage>)

#endif // AQSIS_EQSL_H_INCLUDED
