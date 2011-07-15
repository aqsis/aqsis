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

#include "piqsl2.h"

#include <aqsis/aqsis.h>

#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QListView>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>
#include <QtGui/QSplitter>
#include <QtGui/QStringListModel>

#include <QtGui/QGridLayout>
#include <QtGui/QScrollBar>
#include <QtGui/QFrame>

#include <QtGui/QPainter>
#include <QtGui/QImage>

#include <boost/thread.hpp>

#include <aqsis/version.h>
#include <aqsis/math/math.h>


namespace Aqsis {

PiqslMainWindow::PiqslMainWindow()
{
    setWindowTitle("piqsl");

    // File menu
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    {
        QAction* a = fileMenu->addAction(tr("&Open Library"));
        a->setShortcuts(QKeySequence::Open);
        connect(a, SIGNAL(triggered()), this, SLOT(FIXME()));
    }
    {
        QAction* a = fileMenu->addAction(tr("&Save Library"));
        a->setShortcuts(QKeySequence::Save);
        connect(a, SIGNAL(triggered()), this, SLOT(FIXME()));
    }
    {
        QAction* a = fileMenu->addAction(tr("Save Library &As"));
        a->setShortcuts(QKeySequence::SaveAs);
        connect(a, SIGNAL(triggered()), this, SLOT(FIXME()));
    }
    fileMenu->addSeparator();
    {
        QAction* a = fileMenu->addAction(tr("&Quit"));
        a->setStatusTip(tr("Quit piqsl"));
        a->setShortcuts(QKeySequence::Quit);
        connect(a, SIGNAL(triggered()), this, SLOT(close()));
    }

    // Image menu
    QMenu* imageMenu = menuBar()->addMenu(tr("&Image"));
    {
        QAction* a = imageMenu->addAction(tr("&Add"));
        a->setShortcut(tr("Shift+Ctrl+O"));
        connect(a, SIGNAL(triggered()), this, SLOT(FIXME()));
    }
    {
        QAction* a = imageMenu->addAction(tr("&Remove"));
        a->setShortcut(tr("Shift+Ctrl+X"));
        connect(a, SIGNAL(triggered()), this, SLOT(FIXME()));
    }

    // Help menu
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    {
        QAction* a = helpMenu->addAction(tr("&About"));
        connect(a, SIGNAL(triggered()), this, SLOT(aboutDialog()));
    }

    // Main window is an image viewer and a list of images split by a
    // draggable border.
    QSplitter* splitter = new QSplitter(this);
    setCentralWidget(splitter);

    // List of images
    ImageListModel* imageList = new ImageListModel(this);
    QListView* imageListView = new QListView();
    //imageListView->setDragDropMode(QAbstractItemView::InternalMove);
    imageListView->setItemDelegate(new ImageListDelegate(this));
    splitter->addWidget(imageListView);
    imageListView->setModel(imageList);

    // Image viewer
    PiqslImageView* image = new PiqslImageView();
    splitter->addWidget(image);

    // Collapse list of images for maximum screen space
    QList<int> sizes;
    sizes << 0 << 1;
    splitter->setSizes(sizes);
}


void PiqslMainWindow::keyReleaseEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_Escape)
        close();
    else
        event->ignore();
}


QSize PiqslMainWindow::sizeHint() const
{
    // Size hint, mainly for getting the initial window size right.
    // setMinimumSize() also sort of works for this, but doesn't allow
    // the user to later make the window smaller.
    return QSize(640,480);
}


void PiqslMainWindow::aboutDialog()
{
    QString message = tr(
            "<p><b>piqsl</b> is the aqsis framebuffer and image viewer.</p>"
            "<p>aqsis version %1,<br/>"
            "compiled on %2 at %3.</p>"
            "<p>See <a href=\"http://www.aqsis.org\">http://www.aqsis.org</a>"
            " for more information.</p>"
            ).arg(AQSIS_VERSION_STR_FULL)
            .arg(__DATE__).arg(__TIME__);
    QMessageBox::information(this, tr("About piqsl"), message);
}



//------------------------------------------------------------------------------
PiqslImageView::PiqslImageView(QWidget* parent)
    : QWidget(parent),
    m_image(new CqImage()),
    m_zoom(1),
    m_x(0),
    m_y(0)
{
    setFocusPolicy(Qt::StrongFocus);
    m_image->loadFromFile("lena_std.tif");
}


void PiqslImageView::keyPressEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_H)
    {
        m_x = width()/2 - m_image->imageWidth()*m_zoom/2;
        m_y = height()/2 - m_image->imageHeight()*m_zoom/2;
        update();
    }
    else
        event->ignore();
}


void PiqslImageView::mousePressEvent(QMouseEvent* event)
{
    m_lastPos = event->pos();
}


void PiqslImageView::mouseMoveEvent(QMouseEvent* event)
{
    QPoint delta = event->pos() - m_lastPos;
    m_lastPos = event->pos();
    m_x += delta.x();
    m_y += delta.y();
    update();
}


void PiqslImageView::wheelEvent(QWheelEvent* event)
{
    int newZoom = 0;
    // Zoom in powers of two.
    if(event->delta() > 0)
        newZoom = std::min(1024, 2*m_zoom);
    else
        newZoom = std::max(1, m_zoom/2);
    // Adjust image position so that we zoom in toward the current mouse
    // cursor location
    float ratio = float(newZoom)/m_zoom;
    m_x = lround(event->x() - ratio*(event->x() - m_x));
    m_y = lround(event->y() - ratio*(event->y() - m_y));
    m_zoom = newZoom;
    update();
}


void PiqslImageView::resizeEvent(QResizeEvent* event)
{
    if(!event->oldSize().isValid())
        return;
    // Reposition image such that the pixel in the center of the widget stays
    // in the center after the resize.
    m_x += (event->size().width() - event->oldSize().width())/2;
    m_y += (event->size().height() - event->oldSize().height())/2;
}


void PiqslImageView::paintEvent(QPaintEvent* event)
{
    // With & height of zoomed input
    int wIn = m_image->imageWidth()*m_zoom;
    int hIn = m_image->imageHeight()*m_zoom;
    // Top left location for zoomed input (centred)
    int xIn = m_x;//width()/2 - wIn/2;
    int yIn = m_y;//height()/2 - hIn/2;
    // Clamp source image bound to widget extent
    int x1 = clamp(xIn, 0, width());
    int y1 = clamp(yIn, 0, height());
    int x2 = clamp(xIn + wIn, 0, width());
    int y2 = clamp(yIn + hIn, 0, height());
    // Size of block of pixels
    int w = x2-x1;
    int h = y2-y1;
    if(w <= 0 || h <= 0)
        return;
    int xOff = std::max(0, -xIn);
    int yOff = std::max(0, -yIn);
    QPainter painter(this);
    const CqMixedImageBuffer* buf = m_image->displayBuffer().get();
    QImage img(buf->rawData(), buf->width(), buf->height(), QImage::Format_RGB888);
    painter.drawImage(QRectF(x1, y1, w, h), img,
                      QRectF(float(xOff)/m_zoom, float(yOff)/m_zoom,
                             float(w)/m_zoom, float(h)/m_zoom));
}


//------------------------------------------------------------------------------
ImageListModel::ImageListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_images.push_back(boost::shared_ptr<CqImage>(new CqImage()));
    m_images[0]->loadFromFile("lena_std.tif");
    m_images[0]->setName("lena_std.tif");
    m_images.push_back(boost::shared_ptr<CqImage>(new CqImage()));
    m_images[1]->loadFromFile("menger_glossy.tif");
    m_images[1]->setName("menger_glossy.tif");
}


int ImageListModel::rowCount(const QModelIndex& parent) const
{
    return m_images.size();
}


QVariant ImageListModel::data(const QModelIndex & index, int role) const
{
    if(!index.isValid() || role != Qt::DisplayRole)
        return QVariant();
    if(index.row() >= (int)m_images.size())
        return QVariant();

    return QVariant::fromValue(m_images[index.row()]);
}


//------------------------------------------------------------------------------
ImageListDelegate::ImageListDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void ImageListDelegate::paint(QPainter* painter,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& index) const
{
    if(option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());
    boost::shared_ptr<Aqsis::CqImage> img =
        index.data().value<boost::shared_ptr<Aqsis::CqImage> >();
    // Draw thumbnail
    int thumbW = std::min(option.rect.height(), option.rect.width());
    QRect thumbRect(option.rect.left()+1, option.rect.top()+1,
                    thumbW-2, thumbW-2);
    const CqMixedImageBuffer* buf = img->displayBuffer().get();
    QImage qimg(buf->rawData(), buf->width(), buf->height(), QImage::Format_RGB888);
    painter->drawImage(thumbRect, qimg, QRect(0,0, buf->width(), buf->height()));
    // Draw filename
    QRect textRect(option.rect.left() + thumbW + 1, option.rect.top(),
             option.rect.width(), option.rect.height());
    painter->drawText(textRect, 0, QString::fromStdString(img->name()));
}


} // namespace Aqsis

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    Aqsis::PiqslMainWindow window;
    window.show();

    return app.exec();
}
