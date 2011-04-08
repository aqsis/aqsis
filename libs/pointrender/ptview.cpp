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


#define GL_GLEXT_PROTOTYPES

#include <QtGui/QApplication>

#include "ptview.h"
#include "cornellbox.h"


//------------------------------------------------------------------------------
PointViewport::PointViewport(QWidget *parent)
    : QGLWidget(parent),
    m_prev_x(0),
    m_prev_y(0),
    m_zooming(false),
    m_theta(0),
    m_phi(0),
    m_dist(5),
    m_centre(0)
{
    setFocusPolicy(Qt::StrongFocus);
}


void PointViewport::initializeGL()
{
    // background colour
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    // set up Z buffer
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}


void PointViewport::resizeGL(int w, int h)
{
    // Draw on full window
    glViewport(0, 0, w, h);
    // Set camera projection
    glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        const float n = 0.1f;
        const float hOn2 = 0.5f*n;
        const float wOn2 = hOn2*width()/height();
        glFrustum(-wOn2, wOn2, -hOn2, hOn2, n, 1000);
        glScalef(1, 1, -1);
    glMatrixMode(GL_MODELVIEW);
}


void PointViewport::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Camera -> world transform.  Need to do this here rather than in
    // GL_PROJECTION mode, otherwise the point size scaling won't work
    // correctly.  (Standard GL practise?)
    glLoadIdentity();
    glTranslatef(0, 0, m_dist);
    glRotatef(m_theta, 1, 0, 0);
    glRotatef(m_phi, 0, 1, 0);
    glTranslatef(m_centre.x, m_centre.y, m_centre.z);

    glEnable(GL_MULTISAMPLE);

    // Draw axes
    glColor3f(1.0,0.0,1.0);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(1);
    glColor3f(1.0, 0.7, 0.7);
    glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(1, 0, 0);
    glEnd();
    glColor3f(0.7, 1.0, 0.7);
    glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 1, 0);
    glEnd();
    glColor3f(0.7, 0.7, 1.0);
    glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, 1);
    glEnd();

    // Generate points.  TODO - store these as instance data
    std::vector<float> ptData;
    int ptStride;
    cornellBoxPoints(ptData, ptStride, 5);

    // Draw points
    glPointSize(10);
    glColor3f(1,1,1);
    // Set distance attenuation for points, following the usual 1/z law.
    GLfloat attenParams[3] = {0, 0, 1};
    glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, attenParams);
    glPointParameterf(GL_POINT_SIZE_MIN, 0);
    glPointParameterf(GL_POINT_SIZE_MAX, 100);
    // Scale down model - the Cornell box is measured in mm.
    glTranslatef(-2.5, -2.5, -2.5);
    glScalef(0.01, 0.01, 0.01);
    // Draw all points at once using vertex arrays.
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, ptStride*sizeof(float), &ptData[0]);
    glColorPointer(3, GL_FLOAT, ptStride*sizeof(float), &ptData[0]+7);
    glDrawArrays(GL_POINTS, 0, ptData.size()/ptStride);

    glDisable(GL_MULTISAMPLE);
}


void PointViewport::mousePressEvent(QMouseEvent* event)
{
    m_zooming = event->button() == Qt::RightButton;
    m_prev_x = event->x();
    m_prev_y = event->y();
}


void PointViewport::mouseMoveEvent(QMouseEvent* event)
{
    int dx = m_prev_x - event->x();
    int dy = m_prev_y - event->y();
    m_prev_x = event->x();
    m_prev_y = event->y();
    if(m_zooming)
    {
        m_dist *= std::pow(0.9, 30*double(dy)/height());
    }
    else
    {
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
    }
    repaint();
}


void PointViewport::wheelEvent(QWheelEvent* event)
{
    double degrees = event->delta()/8.0;
    double steps = degrees / 15.0;
    m_dist *= std::pow(0.9, steps);
    repaint();
}


void PointViewport::keyPressEvent(QKeyEvent *event)
{
    event->ignore();
}


//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Turn on multisampled antialiasing - this makes rendered point clouds
    // look much nicer.
    QGLFormat f = QGLFormat::defaultFormat();
    f.setSampleBuffers(true);
    QGLFormat::setDefaultFormat(f);

    PointViewerWindow window;
    window.show();

    return app.exec();
}

// vi: set et:
