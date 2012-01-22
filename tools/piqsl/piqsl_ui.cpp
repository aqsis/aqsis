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

#include "piqsl_ui.h"

#include <aqsis/aqsis.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>
#include <QtGui/QSplitter>
//#include <QtGui/QStringListModel>

#include <QtGui/QPainter>
#include <QtGui/QImage>

#include <boost/thread.hpp>

#include <aqsis/version.h>
#include <aqsis/math/math.h>

#include "imagelistmodel.h"

namespace Aqsis {

PiqslMainWindow::PiqslMainWindow(const QString& socketInterface,
                            int socketPort, const QStringList& filesToOpen)
    : m_currentDirectory("."),
    m_imageList(0),
    m_imageListView(0),
    m_currentLibraryName()
{
    setWindowTitle("piqsl");
	
	Aqsis::CqSocket::initialiseSockets();

    // File menu
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    {
        QAction* a = fileMenu->addAction(tr("&Open Library"));
        a->setShortcuts(QKeySequence::Open);
        connect(a, SIGNAL(triggered()), this, SLOT(openLibrary()));
    }
    {
        QAction* a = fileMenu->addAction(tr("&Save Library"));
        a->setShortcuts(QKeySequence::Save);
        connect(a, SIGNAL(triggered()), this, SLOT(saveLibrary()));
    }
    {
        QAction* a = fileMenu->addAction(tr("Save Library &As"));
        a->setShortcuts(QKeySequence::SaveAs);
        connect(a, SIGNAL(triggered()), this, SLOT(saveLibraryAs()));
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
        connect(a, SIGNAL(triggered()), this, SLOT(addImages()));
    }
    {
        QAction* a = imageMenu->addAction(tr("&Remove"));
        a->setShortcut(tr("Shift+Ctrl+X"));
        connect(a, SIGNAL(triggered()), this, SLOT(removeImage()));
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
    // TODO: Allow interface:port to be set
    m_imageList = new ImageListModel(this, socketInterface, socketPort);
    m_imageListView = new PiqslListView();
    // Attempt at drag/drop stuff, needs work.
//    m_imageListView->setSelectionMode(QAbstractItemView::SingleSelection);
//    m_imageListView->setDragDropMode(QAbstractItemView::InternalMove);
//    m_imageListView->setDragEnabled(true);
//    m_imageListView->setAcceptDrops(true);
//    m_imageListView->setDropIndicatorShown(true);
//    m_imageListView->setMovement(QListView::Snap);
    m_imageListView->setItemDelegate(new ImageListDelegate(this));
    splitter->addWidget(m_imageListView);
    m_imageListView->setModel(m_imageList);

    // Image viewer
    PiqslImageView* image = new PiqslImageView();
    image->setSelectionModel(m_imageListView->selectionModel());
    splitter->addWidget(image);

    // Load in images or image libraries specified at the command line.
    for(int i = 0; i < filesToOpen.size(); ++i)
    {
        QString fileName = filesToOpen[i];
        if(fileName.endsWith(".bks"))
            m_imageList->appendImageLibrary(fileName);
        else
            m_imageList->loadImageFiles(QStringList(fileName));
    }

    // Make list of images small thin to maximize image view space
    QList<int> sizes;
    sizes << 1 << 1000;
    splitter->setSizes(sizes);

    // Test of drag + drop behaviour.
//    QStringListModel* mod = new QStringListModel(this);
//    QStringList lst;
//    lst << "asdf" << "123" << "qwer";
//    mod->setStringList(lst);
//    QListView* lv = new QListView();
//    lv->setModel(mod);
//    lv->setViewMode(QListView::ListMode);
//    lv->setMovement(QListView::Snap);
//    lv->setDragDropMode(QAbstractItemView::InternalMove);
//    lv->setDragDropOverwriteMode(false);
//
//    splitter->addWidget(lv);
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


void PiqslMainWindow::addImages()
{
    QFileDialog fileDialog(this, tr("Select image files"), m_currentDirectory,
                           tr("Image files (*.tif *.tiff *.exr *.env "
                              "*.tx *.tex *.shad *.sm *.map *.zfile *.z)"
                              ";; All files (*.*)"));
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setViewMode(QFileDialog::Detail);
    if(!fileDialog.exec())
        return;
    m_currentDirectory = fileDialog.directory().absolutePath();
    m_imageList->loadImageFiles(fileDialog.selectedFiles());
}


void PiqslMainWindow::removeImage()
{
    if(!m_imageListView->selectionModel()->hasSelection())
        return;
    QModelIndexList selected =
        m_imageListView->selectionModel()->selectedIndexes();
    for(int i = 0; i < selected.size(); ++i)
        m_imageList->removeRows(selected[i].row(), 1);
}


void PiqslMainWindow::openLibrary()
{
    QFileDialog fileDialog(this, tr("Open Library"), m_currentDirectory,
                           tr("Piqsl book files (*.bks)"));
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setViewMode(QFileDialog::Detail);
    if(!fileDialog.exec())
        return;
    m_currentDirectory = fileDialog.directory().absolutePath();
    QStringList selected = fileDialog.selectedFiles();
    if(selected.empty())
        return;
    m_imageList->appendImageLibrary(selected[0]);
}


void PiqslMainWindow::saveLibrary()
{
    if(!m_currentLibraryName.isEmpty())
        m_imageList->saveImageLibrary(m_currentLibraryName);
    else
        saveLibraryAs();
}


void PiqslMainWindow::saveLibraryAs()
{
    QFileDialog fileDialog(this, tr("Save Library As"),
                           m_currentDirectory, tr("Piqsl book files (*.bks)"));
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    if(!fileDialog.exec())
        return;
    m_currentDirectory = fileDialog.directory().absolutePath();
    QStringList selected = fileDialog.selectedFiles();
    if(selected.empty())
        return;
    if(m_imageList->saveImageLibrary(selected[0]))
        m_currentLibraryName = selected[0];
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
    m_image(),
    m_zoom(1),
    m_tlPos(0,0),
    m_lastPos(0,0)
{
    setMinimumSize(QSize(100, 100));
}


void PiqslImageView::keyPressEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_H)
        centerImage();
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
    m_tlPos += QPointF(delta);
    update();
}


void PiqslImageView::wheelEvent(QWheelEvent* event)
{
    float newZoom = 0;
    // Zoom in powers of two.
    if(event->delta() > 0)
        newZoom = std::min(1024.0f, 2*m_zoom);
    else
        newZoom = std::max(1.0f, m_zoom/2);
    // Adjust image position so that we zoom in toward the current mouse
    // cursor location
    QPointF mousePos = event->pos();
    m_tlPos = mousePos - (newZoom/m_zoom)*(mousePos - m_tlPos);
    m_zoom = newZoom;
    update();
}


void PiqslImageView::resizeEvent(QResizeEvent* event)
{
    if(!event->oldSize().isValid())
        return;
    // Reposition image such that the pixel in the center of the widget stays
    // in the center after the resize.
    QPointF size(event->size().width(), event->size().height());
    QPointF oldSize(event->oldSize().width(), event->oldSize().height());
    m_tlPos += 0.5f*(size - oldSize);
}


void PiqslImageView::paintEvent(QPaintEvent* event)
{
    if(!m_image)
    {
        QPainter painter(this);
        painter.drawText(0, 0, width(), height(), Qt::AlignCenter,
                         tr("No image"));
        return;
    }
    int zoom = lround(m_zoom);
    assert(zoom > 0);
    // With & height of zoomed input
    int wIn = m_image->imageWidth()*zoom;
    int hIn = m_image->imageHeight()*zoom;
    // Top left location for zoomed input
    int x0 = lround(m_tlPos.x());
    int y0 = lround(m_tlPos.y());
    // Top left of actual image (may be different from x0,y0 if cropped)
    int xIn = x0 + zoom*m_image->originX();
    int yIn = y0 + zoom*m_image->originY();
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
    const QImage& img = m_image->displayBuffer();
    if(!img.isNull())
    {
        // Draw border for cropping
        painter.drawRect(QRect(x0, y0, m_image->frameWidth()*zoom - 1,
                            m_image->frameHeight()*zoom - 1));
        // Draw appropriate portion of the image
        painter.drawImage(QRectF(x1, y1, w, h), img,
                          QRectF(float(xOff)/zoom, float(yOff)/zoom,
                                 float(w)/zoom, float(h)/zoom));
    }
}


void PiqslImageView::setSelectionModel(QItemSelectionModel* selectionModel)
{
    connect(selectionModel, SIGNAL(selectionChanged(const QItemSelection&,
                                                    const QItemSelection&)),
            this, SLOT(changeSelectedImage(const QItemSelection&,
                                           const QItemSelection&)));
    setSelectedImage(selectionModel->selectedIndexes());
}


void PiqslImageView::changeSelectedImage(const QItemSelection& selected,
                                         const QItemSelection& deselected)
{
    setSelectedImage(selected.indexes());
}


void PiqslImageView::setSelectedImage(const QModelIndexList& indexes)
{
    if(m_image)
        disconnect(m_image.get(), 0, this, 0);
    if(indexes.empty())
    {
        m_image.reset();
        update();
        return;
    }
    int prevWidth = 0;
    int prevHeight = 0;
    if(m_image)
    {
        prevWidth = m_image->frameWidth();
        prevHeight = m_image->frameHeight();
    }
    m_image = indexes[0].data().value<boost::shared_ptr<CqImage> >();
    if(prevWidth > 0 && m_image->frameWidth() > 0)
    {
        // Keep the center of the image in the same place when switching to an
        // image of different dimensions.
        m_tlPos += 0.5f*QPointF(prevWidth - m_image->frameWidth(),
                                prevHeight - m_image->frameHeight());
    }
    else
        centerImage();
    connect(m_image.get(), SIGNAL(updated(int,int,int,int)),
            this, SLOT(imageUpdated(int,int,int,int)));
    connect(m_image.get(), SIGNAL(resized()), this, SLOT(imageResized()));
    update();
}


void PiqslImageView::centerImage()
{
    if(!m_image)
        return;
    m_tlPos = QPointF(width()/2.0f - m_image->frameWidth()*m_zoom/2.0f,
                      height()/2.0f - m_image->frameHeight()*m_zoom/2.0f);
    update();
}


void PiqslImageView::imageUpdated(int x, int y, int w, int h)
{
    int zoom = lround(m_zoom);
    update(lround(m_tlPos.x()) + zoom*x, lround(m_tlPos.y()) + zoom*y,
            zoom*w, zoom*h);
}


void PiqslImageView::imageResized()
{
    centerImage();
}


//------------------------------------------------------------------------------
PiqslListView::PiqslListView(QWidget* parent)
    : QListView(parent)
{
}


void PiqslListView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    // If new items are inserted at end, select them.
    if(end == model()->rowCount() - 1)
    {
        clearSelection();
        setCurrentIndex(model()->index(model()->rowCount() - 1, 0));
    }
    QListView::rowsInserted(parent, start, end);
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
    const QImage& qimg = img->displayBuffer();
    if(!qimg.isNull())
        painter->drawImage(thumbRect, qimg, QRect(0,0, qimg.width(), qimg.height()));
    // Draw filename
    QRect textRect(option.rect.left() + thumbW + 1, option.rect.top(),
             option.rect.width(), option.rect.height());
    painter->drawText(textRect, 0, QString::fromStdString(img->name()));
}


} // namespace Aqsis
