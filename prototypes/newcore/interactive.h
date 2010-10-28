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

#include <iostream>
#include <fstream>
#include <cstdlib>

#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>

#include <QtGui>

#include <aqsis/riutil/ricxxutil.h>

#include "api.h"
#include "displaymanager.h"

using namespace Aqsis;

//------------------------------------------------------------------------------
class QtDisplay : public Aqsis::Display
{
    public:
        QtDisplay(QImage& image)
            : m_image(image)
        { }

        virtual bool open(const std::string& fileName, const V2i& imageSize,
                          const V2i& tileSize, const VarSpec& varSpec)
        {
            m_imageSize = imageSize;
            m_tileSize = tileSize;
            return true;
        }

        virtual bool writeTile(const V2i& pos, void* data)
        {
            int nchans = 3;
            int tileRowSize = nchans*m_tileSize.x;
            // Clamp output tile size to extent of image.
            V2i outTileEnd = min(pos + m_tileSize, m_imageSize);
            V2i outTileSize = outTileEnd - pos;
            int outTileRowSize = sizeof(uchar)*nchans*outTileSize.x;
            // Copy data.  Note that QImage aligns scanlines to 32-bit
            // boundaries, so adjacent scanlines may have padding between them.
            const uchar* src = (const uchar*)data;
            for(int i = 0; i < outTileSize.y; ++i)
            {
                uchar* dest = m_image.scanLine(i + pos.y) + nchans*pos.x;
                std::memcpy(dest, src, outTileRowSize);
                src += tileRowSize;
            }
            return true;
        }

        virtual bool close()
        {
            return true;
        }

    private:
        V2i m_imageSize;
        V2i m_tileSize;
        QImage& m_image;
};

//------------------------------------------------------------------------------
class InteractiveRender : public QWidget
{
	Q_OBJECT

    public:
		InteractiveRender(int x, int y, V2i imageSize, Ri::RendererServices& renderer, std::vector<std::string>& retainedModels)
		:	m_prev_x(0),
            m_prev_y(0),
			m_mouseDrag(false),
            m_theta(0),
            m_phi(0),
            m_dist(5),
            m_centre(0,0,0),
            m_imageSize(V2i(0)),
            m_qtImage(),
            m_renderer(renderer),
            m_display(m_qtImage),
			m_retainedModels(retainedModels),
			m_rm(m_retainedModels.begin())
        {
            setImageSize(imageSize);

            Ri::Renderer& ri = renderer.firstFilter();
            // Set up the display
            Aqsis::Display* disp = &m_display;
            ri.Display("Ci.tif", "__Display_instance__", "rgb",
                       ParamListBuilder()("pointer instance", &disp));
			QTimer *timer = new QTimer(this);
			connect(timer, SIGNAL(timeout()), this, SLOT(tick()));
			timer->start(40);
            // Kick off initial render
            renderImage();
        }

	protected:
		void paintEvent(QPaintEvent* )
		{
			QPainter qpainter(this);

			qpainter.drawImage(rect(), m_qtImage, rect());
		}

		void resizeEvent(QResizeEvent* event)
		{
			setImageSize(V2i(event->size().width(), event->size().height()));
			renderImage();
		}

		void mousePressEvent(QMouseEvent* event)
		{
			m_prev_x = event->x();
			m_prev_y = event->y();
			m_mouseDrag = true;
		}

		void mouseReleaseEvent(QMouseEvent* event)
		{
			m_mouseDrag = false;
		}

		void mouseMoveEvent(QMouseEvent* event)
		{
			int dx = m_prev_x - event->x();
			int dy = m_prev_y - event->y();
			m_prev_x = event->x();
			m_prev_y = event->y();
			if(event->modifiers() & Qt::ControlModifier)
			{
				m_centre.y +=  m_dist/height()*dy;
				m_centre.x += -m_dist/width()*dx*std::cos(deg2rad(m_phi));
				m_centre.z += -m_dist/width()*dx*std::sin(deg2rad(m_phi));
			}
			else
			{
				m_theta += 0.5*dy;
				m_phi   += 0.5*dx;
			}
			renderImage();
		}

		void wheelEvent(QWheelEvent* event)
		{
			double degrees = event->delta()/8.0;
			double steps = degrees / 15.0;
			m_dist *= std::pow(0.9, steps);
			renderImage();
		}

	public slots:
		void tick()
		{
			++m_rm;
			if(m_rm == m_retainedModels.end())
				m_rm = m_retainedModels.begin();
			renderImage();
		}

    private:
        void setImageSize(V2i imageSize)
        {
            m_imageSize = imageSize;
			m_qtImage = QImage(imageSize.x, imageSize.y, QImage::Format_RGB888);
			m_qtImage.fill(0xFF0000);
        }

        void renderImage()
        {
            Ri::Renderer& ri = m_renderer.firstFilter();

            ri.FrameBegin(1);

            ri.Format(m_imageSize.x, m_imageSize.y, 1);

            // Viewing transformation
            float fov = 90;
            ri.Projection("perspective",
                          ParamListBuilder()("float fov", &fov));
            ri.Translate(0, 0, m_dist);
            ri.Rotate(m_theta, 1, 0, 0);
            ri.Rotate(m_phi, 0, 1, 0);
            ri.Translate(m_centre.x, m_centre.y, m_centre.z);

            ri.WorldBegin();
            ri.ReadArchive(m_rm->c_str(), 0, ParamListBuilder());
            ri.WorldEnd();
            ri.FrameEnd();

			repaint();
        }

        int m_prev_x;
        int m_prev_y;
		bool m_mouseDrag;

        float m_theta;
        float m_phi;
        float m_dist;
        Vec3 m_centre;
        V2i m_imageSize;
		QImage m_qtImage;

        Ri::RendererServices& m_renderer;
        QtDisplay m_display;

		std::vector<std::string> m_retainedModels;
		std::vector<std::string>::iterator m_rm;
};


//------------------------------------------------------------------------------
class RenderWindow : public QMainWindow
{
	Q_OBJECT

    public:
		RenderWindow(int w, int h, Ri::RendererServices& renderer, std::vector<std::string>& retainedModels)
        {
            m_renderWidget = new InteractiveRender(x(), y(), V2i(w,h), renderer, retainedModels);
            m_renderWidget->setMinimumSize(QSize(w, h));
            setCentralWidget(m_renderWidget);
            setWindowTitle("Aqsis-2.0 demo");
        }

	signals:
		void exitApplication();

	protected:
		void keyReleaseEvent(QKeyEvent* event)
		{
			if(event->key() == Qt::Key_Escape)
			{
				emit(exitApplication());
			}
		}

    private:
        InteractiveRender* m_renderWidget;
};


