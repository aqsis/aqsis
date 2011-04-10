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

#ifndef AQSIS_PTVIEW_H_INCLUDED
#define AQSIS_PTVIEW_H_INCLUDED

#include <cmath>

#include <QtGui/QMainWindow>
#include <QtGui/QKeyEvent>
#include <QtOpenGL/QGLWidget>

#include <OpenEXR/ImathVec.h>

#include "pointcloud.h"


using Imath::V3f;

inline float deg2rad(float d) { return (M_PI/180) * d; }


//------------------------------------------------------------------------------
/// OpenGL-based viewer widget for point clouds (or more precisely, clouds of
/// disk-like surface elements).
class PointView : public QGLWidget
{
    Q_OBJECT

    public:
        /// Point (surface element) visualization mode
        enum VisMode
        {
            Vis_Points,  ///< Draw surfels using GL_POINTS
            Vis_Disks    ///< Draw surfels as disks
        };

        PointView(QWidget *parent = NULL);

        /// Set points to be rendered
        void setPoints(const boost::shared_ptr<const PointArray>& points);

    protected:
        // Qt OpenGL callbacks
        void initializeGL();
        void resizeGL(int w, int h);
        void paintGL();

        // Qt event callbacks
        void mousePressEvent(QMouseEvent* event);
        void mouseMoveEvent(QMouseEvent* event);
        void wheelEvent(QWheelEvent* event);
        void keyPressEvent(QKeyEvent *event);

    private:
        /// Draw point cloud using OpenGL
        static void drawPoints(const PointArray& points, VisMode visMode);

        /// Mouse-based camera positioning
        int m_prev_x;
        int m_prev_y;
        bool m_zooming;
        float m_theta;
        float m_phi;
        float m_dist;
        V3f m_centre;
        /// Type of visualization
        VisMode m_visMode;
        /// Point cloud data
        boost::shared_ptr<const PointArray> m_points;
        V3f m_cloudCenter;
};


//------------------------------------------------------------------------------
/// Main window for point cloud viewer application
class PointViewerWindow : public QMainWindow
{
    Q_OBJECT

    public:
        PointViewerWindow()
        {
            m_pointView = new PointView();
            m_pointView->setMinimumSize(QSize(640, 480));
            setCentralWidget(m_pointView);
            setWindowTitle("Aqsis point cloud viewer");
        }

        PointView& pointView() { return *m_pointView; }

    protected:
        void keyReleaseEvent(QKeyEvent* event)
        {
            if(event->key() == Qt::Key_Escape)
                close();
        }

    private:
        PointView* m_pointView;
};



#endif // AQSIS_PTVIEW_H_INCLUDED

// vi: set et:
