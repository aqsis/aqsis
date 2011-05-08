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

#include <boost/program_options.hpp>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathGL.h>

#include "ptview.h"
#include "cornellbox.h"
#include "microbuffer.h"

namespace Aqsis {

inline float rad2deg(float r)
{
    return r*180/M_PI;
}

/// Get max and min of depth buffer, ignoring any depth > FLT_MAX/2
///
/// \param z - depth array
/// \param size - length of z array
/// \param stride - stride between values in z array
/// \param zMin - returned min z value
/// \param zMax - returned max z value
static void depthRange(const float* z, int size, int stride,
                       float& zMin, float& zMax)
{
    zMin = FLT_MAX;
    zMax = -FLT_MAX;
    for(int i = 0; i < size; ++i)
    {
        float zi = z[i*stride];
        if(zMin > zi)
            zMin = zi;
        if(zMax < zi && zi < FLT_MAX/2)
            zMax = zi;
    }
}


#if 0
static void drawBound(const Box3f& b);
static void drawDisk(V3f p, V3f n, float r)
{
    // Radius 1 disk primitive, translated & scaled into position.
    V3f p0(0);
    V3f t1(1,0,0);
    V3f t2(0,1,0);
    V3f diskNormal(0,0,1);
    glPushMatrix();
    // Translate disk to point location and scale
    glTranslate(p);
    glScalef(r,r,r);
    // Transform the disk normal (0,0,1) into the correct normal
    // direction for the current point.  The appropriate transform
    // is a rotation about a direction perpendicular to both
    // normals:
    V3f v = diskNormal % n;
    // via the angle given by the dot product:
    float angle = rad2deg(acosf(diskNormal^n));
    glRotatef(angle, v.x, v.y, v.z);
    glColor3f(1, 0, 0);
    glBegin(GL_LINE_LOOP);
        const int nSegs = 20;
        // Scale so that _min_ radius of polygon approx is 1.
        float scale = 1/cos(M_PI/nSegs);
        for(int i = 0; i < nSegs; ++i)
        {
            float angle = 2*M_PI*float(i)/nSegs;
            glVertex3f(scale*cos(angle), scale*sin(angle), 0);
        }
    glEnd();
    glPopMatrix();
}


/// Debug: visualize tree splitting
static void splitNode(V3f P, float maxSolidAngle, int dataSize,
                      const PointOctree::Node* node)
{
    glColor3f(0.3, 0.5, 0.3);
    drawBound(node->bound);
    if(node->npoints != 0)
    {
        // Render each point.
        for(int i = 0; i < node->npoints; ++i)
        {
            const float* data = &node->data[i*dataSize];
            V3f p = V3f(data[0], data[1], data[2]);
            V3f n = V3f(data[3], data[4], data[5]);
            float r = data[6];
            drawDisk(p, n, r);
        }
    }
    for(int i = 0; i < 8; ++i)
    {
        PointOctree::Node* child = node->children[i];
        if(!child)
            continue;
        // Check size of the child node
        float rbnd = child->aggR;
        V3f pbnd = child->aggP - P;
        float pbnd2 = pbnd.length2();
        float solidAngle = M_PI*rbnd*rbnd / pbnd2;
        if(solidAngle < maxSolidAngle)
            drawDisk(child->aggP, child->aggN, child->aggR);
        else
            splitNode(P, maxSolidAngle, dataSize, child);
    }
}
#endif


/// Convert depth buffer to grayscale color
///
/// \param z - depth buffer
/// \param size - length of z array
/// \param col - storage for output RGB tripls color data
static void depthToColor(const float* z, int size, int stride, GLubyte* col,
                         float zMin, float zMax)
{
    float zRangeInv = 1.0f/(zMax - zMin);
    for(int i = 0; i < size; ++i)
    {
        GLubyte c = Imath::clamp(int(255*(1 - zRangeInv*(z[stride*i] - zMin))),
                                 0, 255);
        col[3*i] = c;
        col[3*i+1] = c;
        col[3*i+2] = c;
    }
}

/// Convert coverage grayscale color
static void coverageToColor(const float* face, int size, int stride, GLubyte* col)
{
    for(int i = 0; i < size; ++i)
    {
        GLubyte c = Imath::clamp(int(255*(face[stride*i])), 0, 255);
        col[3*i] = c;
        col[3*i+1] = c;
        col[3*i+2] = c;
    }
}

static void floatColToColor(const float* face, int size, int stride, GLubyte* col)
{
    for(int i = 0; i < size; ++i)
    {
        col[3*i]   = Imath::clamp(int(255*(face[stride*i])), 0, 255);
        col[3*i+1] = Imath::clamp(int(255*(face[stride*i+1])), 0, 255);
        col[3*i+2] = Imath::clamp(int(255*(face[stride*i+2])), 0, 255);
    }
}

/// Draw face of a cube environment map.
///
/// \param p - origin of 1x1 quad
/// \param cols - square texture of RGB triples of size width*width
/// \param width - side length of cols texture
static void drawCubeEnvFace(Imath::V2f p, GLubyte* cols, int colsWidth)
{
    // Set up texture
    GLuint texName = 0;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);
    // Set texture wrap modes to clamp
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    // Set filtering to nearest neighbour
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // Send texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, colsWidth, colsWidth, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, cols);
    // Enable texturing
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture(GL_TEXTURE_2D, texName);
    // Draw quad
    glPushMatrix();
    glTranslatef(p.x, p.y, 0);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(0, 0);
        glTexCoord2f(1, 0); glVertex2f(1, 0);
        glTexCoord2f(1, 1); glVertex2f(1, 1);
        glTexCoord2f(0, 1); glVertex2f(0, 1);
    glEnd();
    glPopMatrix();
    glDeleteTextures(1, &texName);
}


/// Draw a micro depth environment buffer using OpenGL
///
/// The cube faces are laid out on screen as follows:
///
///         +---+
///         |+y |
/// +---+---+---+---+
/// |-z |-x |+z |+x |
/// +---+---+---+---+
///         |-y |
///         +---+
///
static void drawMicroBuf(const MicroBuf& envBuf)
{
    // Make new array of texels.
    int res = envBuf.res();
    int npix = res*res;
    int faceSize = npix*3;
    boost::scoped_array<GLubyte> colBuf(new GLubyte[faceSize*6]);
    // Convert each face to 8-bit colour texels
    if(false)
    {
        float zMin = 0;
        float zMax = 0;
        depthRange(envBuf.face(MicroBuf::Face_xp), npix*6, envBuf.nchans(),
                   zMin, zMax);
        for(int face = 0; face < 6; ++face)
            depthToColor(envBuf.face(face), npix, envBuf.nchans(),
                         &colBuf[faceSize*face], zMin, zMax);
    }
    else if(false)
    {
        for(int face = 0; face < 6; ++face)
            coverageToColor(envBuf.face(face), npix, envBuf.nchans(),
                            &colBuf[faceSize*face]);
    }
    else
    {
        // Convert float face color into 8-bit color texels for OpenGL
        for(int face = 0; face < 6; ++face)
            floatColToColor(envBuf.face(face) + 2, npix, envBuf.nchans(),
                            &colBuf[faceSize*face]);
    }
    // Set up coordinates so we render on a 4x3 grid
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-1, -1, 0);
    glScalef(0.5f, 2.0f/3.0f, 1);

    // Render the six faces in a cross.
    drawCubeEnvFace(V2f(0,1), &colBuf[faceSize*MicroBuf::Face_zn], res);
    drawCubeEnvFace(V2f(1,1), &colBuf[faceSize*MicroBuf::Face_xn], res);
    drawCubeEnvFace(V2f(2,1), &colBuf[faceSize*MicroBuf::Face_zp], res);
    drawCubeEnvFace(V2f(3,1), &colBuf[faceSize*MicroBuf::Face_xp], res);
    drawCubeEnvFace(V2f(2,0), &colBuf[faceSize*MicroBuf::Face_yn], res);
    drawCubeEnvFace(V2f(2,2), &colBuf[faceSize*MicroBuf::Face_yp], res);
}


//------------------------------------------------------------------------------
PointView::PointView(QWidget *parent)
    : QGLWidget(parent),
    m_prev_x(0),
    m_prev_y(0),
    m_zooming(false),
    m_theta(0),
    m_phi(0),
    m_dist(5),
    m_center(0),
    m_probePos(0),
    m_probeMoveMode(false),
    m_probeRes(10),
    m_probeMaxSolidAngle(0),
    m_visMode(Vis_Points),
    m_lighting(false),
    m_points(),
    m_pointTree(0),
    m_cloudCenter(0)
{
    setFocusPolicy(Qt::StrongFocus);
}


void PointView::setPoints(const boost::shared_ptr<const PointArray>& points,
                          const PointOctree* pointTree)
{
    m_points = points;
    if(m_points)
    {
        m_cloudCenter = m_points->centroid();
        m_probePos = m_cloudCenter;
    }
    m_pointTree = pointTree;
}

void PointView::setProbeParams(int cubeFaceRes, float maxSolidAngle)
{
    m_probeRes = cubeFaceRes;
    m_probeMaxSolidAngle = maxSolidAngle;
}


void PointView::initializeGL()
{
    glLoadIdentity();

    // set up Z buffer
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    switch(m_visMode)
    {
        case Vis_Points:
        {
            glDisable(GL_COLOR_MATERIAL);
            glDisable(GL_LIGHT0);
        }
        break;
        case Vis_Disks:
        {
            // Materials
            glShadeModel(GL_SMOOTH);
            if(m_lighting)
            {
                glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
                glEnable(GL_COLOR_MATERIAL);
                // Lighting
                // light0
                GLfloat lightPos[] = {0, 0, 0, 1};
                GLfloat whiteCol[] = {1, 1, 1, 1};
                glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
                glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteCol);
                glEnable(GL_LIGHT0);
                // whole-scene ambient intensity scaling
                GLfloat ambientCol[] = {0.1, 0.1, 0.1, 1};
                glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientCol);
                // whole-scene diffuse intensity
                //GLfloat diffuseCol[] = {0.05, 0.05, 0.05, 1};
                //glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseCol);
                // two-sided lighting.
                // TODO: Why doesn't this work?? (handedness problems?)
                //glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
                // rescaling of normals
                glEnable(GL_RESCALE_NORMAL);
            }
        }
        break;
    }

    glEnable(GL_MULTISAMPLE);
}


void PointView::resizeGL(int w, int h)
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
    glMatrixMode(GL_MODELVIEW);
}


static void drawBound(const Box3f& b)
{
    glBegin(GL_LINE_LOOP);
        glVertex3f(b.min.x, b.min.y, b.min.z);
        glVertex3f(b.min.x, b.min.y, b.max.z);
        glVertex3f(b.max.x, b.min.y, b.max.z);
        glVertex3f(b.max.x, b.min.y, b.min.z);
    glEnd();
    glBegin(GL_LINE_LOOP);
        glVertex3f(b.min.x, b.max.y, b.min.z);
        glVertex3f(b.min.x, b.max.y, b.max.z);
        glVertex3f(b.max.x, b.max.y, b.max.z);
        glVertex3f(b.max.x, b.max.y, b.min.z);
    glEnd();
    glBegin(GL_LINES);
        glVertex3f(b.min.x, b.min.y, b.min.z);
        glVertex3f(b.min.x, b.max.y, b.min.z);
        glVertex3f(b.min.x, b.min.y, b.max.z);
        glVertex3f(b.min.x, b.max.y, b.max.z);
        glVertex3f(b.max.x, b.min.y, b.max.z);
        glVertex3f(b.max.x, b.max.y, b.max.z);
        glVertex3f(b.max.x, b.min.y, b.min.z);
        glVertex3f(b.max.x, b.max.y, b.min.z);
    glEnd();
}


void PointView::paintGL()
{
    RadiosityIntegrator integrator(m_probeRes);
//    if(m_points)
//        microRasterize(integrator, m_probePos, V3f(0,0,-1), M_PI, *m_points);
    if(m_pointTree)
        microRasterize(integrator, m_probePos, V3f(0,0,-1), M_PI,
                       m_probeMaxSolidAngle, *m_pointTree);

    //--------------------------------------------------
    // Draw main scene
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Camera -> world transform.
    glLoadIdentity();
    glScalef(1, 1, -1);
    glTranslatef(0, 0, m_dist);
    glRotatef(m_theta, 1, 0, 0);
    glRotatef(m_phi, 0, 1, 0);
    glTranslate(m_center);
    // Center on the point cloud
    glTranslate(-m_cloudCenter);

    // Geometry
    drawAxes();
//    drawLightProbe(m_probePos, 1 - integrator.occlusion(V3f(0,0,-1)));
    drawLightProbe(m_probePos, integrator.radiosity(V3f(0,0,-1)));
    if(m_points)
        drawPoints(*m_points, m_visMode, m_lighting);
//    if(m_pointTree)
//        splitNode(m_probePos, m_probeMaxSolidAngle, m_pointTree->dataSize(),
//                  m_pointTree->root());


    //--------------------------------------------------
    glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT | GL_ENABLE_BIT);
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_SCISSOR_TEST);
    //--------------------------------------------------
    // Draw scene from position of light probe.
    GLuint miniBufWidth = width()/3;

#if 0
    // Set up & clear viewport
    glScissor(0, 0, miniBufWidth, miniBufWidth);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, miniBufWidth, miniBufWidth);

    // Projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const float n = 0.01f;
    glFrustum(-n, n, -n, n, n, 1000);
    glMatrixMode(GL_MODELVIEW);

    // Camera transform
    glLoadIdentity();
    glScalef(1, 1, -1);
    glRotatef(180, 0, 1, 0);
    glTranslate(-m_probePos);

    // Geometry
    if(m_points)
        drawPoints(*m_points, m_visMode, m_lighting);
#endif

    //--------------------------------------------------
    // Draw image of rendered microbuffer
    glScissor(width() - miniBufWidth, 0, miniBufWidth, miniBufWidth*3/4);
    glViewport(width() - miniBufWidth, 0, miniBufWidth, miniBufWidth*3/4);

    drawMicroBuf(integrator.microBuf());

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib();
}


void PointView::mousePressEvent(QMouseEvent* event)
{
    m_zooming = event->button() == Qt::RightButton;
    m_prev_x = event->x();
    m_prev_y = event->y();
}


void PointView::mouseMoveEvent(QMouseEvent* event)
{
    float dx = float(m_prev_x - event->x())/width();
    float dy = float(m_prev_y - event->y())/height();
    m_prev_x = event->x();
    m_prev_y = event->y();
    if(m_probeMoveMode)
    {
        // Modify the position of the light probe.
        V3f camDir = V3f(cos(deg2rad(m_theta))*cos(deg2rad(m_phi+90)),
                         sin(deg2rad(m_theta)),
                         cos(deg2rad(m_theta))*sin(deg2rad(m_phi+90)));
        V3f camPos = -m_dist*camDir + m_center - m_cloudCenter;
        V3f v = m_probePos - camPos;
        V3f up = V3f(cos(deg2rad(m_theta+90))*cos(deg2rad(m_phi+90)),
                     sin(deg2rad(m_theta+90)),
                     cos(deg2rad(m_theta+90))*sin(deg2rad(m_phi+90)));
        V3f right = camDir % up;
        // Distance along view direction to light probe
        float d = dot(v, camDir);
        if(m_zooming)
            m_probePos += dy*d*camDir;
        else
            m_probePos += dy*d*up + dx*d*right;
    }
    else if(m_zooming)
    {
        // Zoom distance to camera pivot point
        m_dist *= std::pow(0.9, 30*dy);
    }
    else
    {
        // Modify position of centre
        if(event->modifiers() & Qt::ControlModifier)
        {
            m_center.y +=  m_dist*dy;
            m_center.x += -m_dist*dx*std::cos(deg2rad(m_phi));
            m_center.z += -m_dist*dx*std::sin(deg2rad(m_phi));
        }
        else
        {
            // rotate camera
            m_theta += 400*dy;
            m_phi   += 400*dx;
        }
    }
    repaint();
}


void PointView::wheelEvent(QWheelEvent* event)
{
    double degrees = event->delta()/8.0;
    double steps = degrees / 15.0;
    m_dist *= std::pow(0.9, steps);
    repaint();
}


void PointView::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_V)
    {
        m_visMode = (m_visMode == Vis_Points) ? Vis_Disks : Vis_Points;
        initializeGL();
        repaint();
    }
    else if(event->key() == Qt::Key_Z)
        m_probeMoveMode = true;
    else if(event->key() == Qt::Key_L)
    {
        m_lighting = !m_lighting;
        initializeGL();
        repaint();
    }
    else if(event->key() == Qt::Key_C)
    {
        // FIXME: Coord system is a bit screwy here.
        m_center = m_cloudCenter - m_probePos;
        repaint();
    }
    else
        event->ignore();
}

void PointView::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Z)
        m_probeMoveMode = false;
    else
        event->ignore();
}

/// Draw a set of axes
void PointView::drawAxes()
{
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(1);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(1, 0, 0);
    glEnd();
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 1, 0);
    glEnd();
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, 1);
    glEnd();
}


/// Draw position of the light probe
void PointView::drawLightProbe(const V3f& P, const C3f& col)
{
    glColor(col);
    glPointSize(100);
    // Draw point at probe position
    glBegin(GL_POINTS);
        glVertex(P);
    glEnd();
    glColor3f(0.7,0.7,1);
    // Draw axis-aligned box around point
    float r = 0.1;
    drawBound(Box3f(P - r*V3f(1), P + r*V3f(1)));
    // Draw lines from origin to point along axis directions
    glBegin(GL_LINE_STRIP);
        glVertex3f(P.x, P.y, P.z);
        glVertex3f(P.x, 0, P.z);
        glVertex3f(0, 0, P.z);
    glEnd();
    glBegin(GL_LINES);
        glVertex3f(0, 0, P.z);
        glVertex3f(0, 0, 0);
    glEnd();
}


/// Draw point cloud using OpenGL
void PointView::drawPoints(const PointArray& points, VisMode visMode,
                           bool useLighting)
{
    int ptStride = points.stride;
    int npoints = points.data.size()/ptStride;
    const float* ptData = &points.data[0];

    switch(visMode)
    {
        case Vis_Points:
        {
            // Draw points
            glPointSize(1);
            glColor3f(1,1,1);
            // Set distance attenuation for points, following the usual 1/z
            // law.
            GLfloat attenParams[3] = {0, 0, 1};
            glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, attenParams);
            glPointParameterf(GL_POINT_SIZE_MIN, 0);
            glPointParameterf(GL_POINT_SIZE_MAX, 100);
            // Draw all points at once using vertex arrays.
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_COLOR_ARRAY);
            glVertexPointer(3, GL_FLOAT, ptStride*sizeof(float), ptData);
            glColorPointer(3, GL_FLOAT, ptStride*sizeof(float), ptData+7);
            glDrawArrays(GL_POINTS, 0, npoints);
        }
        break;
        case Vis_Disks:
        {
            //glCullFace(GL_BACK);
            //glEnable(GL_CULL_FACE);
            // Compile radius 1 disk primitive
            V3f p0(0);
            V3f t1(1,0,0);
            V3f t2(0,1,0);
            V3f diskNormal(0,0,1);
            GLuint disk = glGenLists(1);
            glNewList(disk, GL_COMPILE);
                glBegin(GL_TRIANGLE_FAN);
                const int nSegs = 20;
                // Scale so that _min_ radius of polygon approx is 1.
                float scale = 1/cos(M_PI/nSegs);
                glVertex3f(0,0,0);
                for(int i = 0; i <= nSegs; ++i)
                {
                    float angle = 2*M_PI*float(i)/nSegs;
                    glVertex3f(scale*cos(angle), scale*sin(angle), 0);
                }
                glEnd();
                // Disk normal
                //glBegin(GL_LINES);
                //    glVertex(V3f(0));
                //    glVertex(diskNormal);
                //glEnd();
            glEndList();
            // Draw points as disks.
            // TODO: Doing this in immediate mode is really slow!  Using
            // vertex shaders would probably be a much better method, or
            // perhaps just compile into a display list?
            if(useLighting)
                glEnable(GL_LIGHTING);
            for(int i = 0; i < npoints; ++i)
            {
                const float* data = ptData + i*ptStride;
                V3f p(data[0], data[1], data[2]);
                V3f n(data[3], data[4], data[5]);
                float r = M_SQRT2*data[6];
                glColor3f(data[7], data[8], data[9]);
                glPushMatrix();
                // Translate disk to point location and scale
                glTranslate(p);
                glScalef(r,r,r);
                // Transform the disk normal (0,0,1) into the correct normal
                // direction for the current point.  The appropriate transform
                // is a rotation about a direction perpendicular to both
                // normals:
                V3f v = diskNormal % n;
                // via the angle given by the dot product:
                float angle = rad2deg(acosf(diskNormal^n));
                glRotatef(angle, v.x, v.y, v.z);
                // Instance the disk
                glCallList(disk);
                glPopMatrix();
            }
            glDeleteLists(disk, 1);
            if(useLighting)
                glDisable(GL_LIGHTING);
        }
        break;
    }
}


} // namespace Aqsis


//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    using namespace Aqsis;

    QApplication app(argc, argv);

    typedef std::vector<std::string> StringVec;
    namespace po = boost::program_options;
    // optional options
    po::options_description optionsDesc("options");
    optionsDesc.add_options()
        ("help,h", "help message")
        ("envres,e", po::value<int>()->default_value(10),
         "resolution of micro environment raster faces for baking")
        ("maxsolidangle,a", po::value<float>()->default_value(0.02),
         "max solid angle used for aggreates in point hierarchy during rendering")
        ("proberes,p", po::value<int>(),
         "resolution of micro environment raster for viewing")
        ("cloudres,c", po::value<float>()->default_value(20),
         "resolution of point cloud")
        ("bakeonly,b", "Only bake, don't display (useful for timing)")
        ("radiusmult,r", po::value<float>()->default_value(1),
         "multiplying factor for surfel radius")
        ("point_file", po::value<std::string>(), "file to display")
    ;
    // Parse options
    po::variables_map opts;
    po::positional_options_description positionalOpts;
    positionalOpts.add("point_file", 1);
    po::store(po::command_line_parser(app.argc(), app.argv())
              .options(optionsDesc).positional(positionalOpts).run(), opts);
    po::notify(opts);

    if(opts.count("help"))
    {
        std::cout << "Usage: " << argv[0] << " [options] file.ptc\n\n"
                  << optionsDesc;
        return 0;
    }

    // Turn on multisampled antialiasing - this makes rendered point clouds
    // look much nicer.
    QGLFormat f = QGLFormat::defaultFormat();
    f.setSampleBuffers(true);
    QGLFormat::setDefaultFormat(f);

    PointViewerWindow window;

    std::cout << "Reading point cloud...  " << std::flush;
    boost::shared_ptr<PointArray> points;
    if(opts.count("point_file") != 0)
        points = loadPointFile(opts["point_file"].as<std::string>());
    else
        points = cornellBoxPoints(opts["cloudres"].as<float>());
    std::cout << "npoints = " << points->size() << "\n";

    std::cout << "Building point hierarchy...\n";
    PointOctree octree(*points);

    int envRes = opts["envres"].as<int>();
    float maxSolidAngle = opts["maxsolidangle"].as<float>();
    std::cout << "Baking...\n";
//    bakeOcclusion(*points, octree, envRes, maxSolidAngle);
    bakeRadiosity(*points, octree, envRes, maxSolidAngle);
    if(opts.count("bakeonly"))
        return 0;

    window.pointView().setPoints(points, &octree);
    int probeRes = envRes;
    if(opts.count("proberes") != 0)
        probeRes = opts["proberes"].as<int>();
    window.pointView().setProbeParams(probeRes, maxSolidAngle);

    window.show();

    return app.exec();
}


// vi: set et:
