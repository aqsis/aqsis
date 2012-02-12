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

#include <QtCore/QSignalMapper>
#include <QtGui/QApplication>
#include <QtGui/QKeyEvent>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QColorDialog>

#include <boost/program_options.hpp>

#define NOMINMAX
#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathMatrix.h>

#include "ptview.h"
#include "microbuffer.h"

#include <aqsis/version.h>

namespace Aqsis {

//----------------------------------------------------------------------
//#include <OpenEXR/ImathGL.h>
// Utilities for OpenEXR / OpenGL interoperability.
//
// Technically we could use the stuff from ImathGL instead here, but it has
// portability problems for OSX due to how it includes gl.h (this is an
// libilmbase bug, at least up until 1.0.2)
inline void glTranslate(const Imath::V3f& v)
{
    glTranslatef(v.x, v.y, v.z);
}

inline void glVertex(const Imath::V3f& v)
{
    glVertex3f(v.x, v.y, v.z);
}

inline void glVertex(const Imath::V2f& v)
{
    glVertex2f(v.x, v.y);
}

inline void glColor(const Imath::C3f& c)
{
    glColor3f(c.x, c.y, c.z);
}

inline void glLoadMatrix(const Imath::M44f& m)
{
    glLoadMatrixf((GLfloat*)m[0]);
}


//----------------------------------------------------------------------
inline float rad2deg(float r)
{
    return r*180/M_PI;
}

inline QVector3D exr2qt(const Imath::V3f& v)
{
    return QVector3D(v.x, v.y, v.z);
}

inline Imath::V3f qt2exr(const QVector3D& v)
{
    return Imath::V3f(v.x(), v.y(), v.z());
}

inline Imath::M44f qt2exr(const QMatrix4x4& m)
{
    Imath::M44f mOut;
    for(int j = 0; j < 4; ++j)
    for(int i = 0; i < 4; ++i)
        mOut[j][i] = m.constData()[4*j + i];
    return mOut;
}


//----------------------------------------------------------------------
// PointViewArray implementation

static void releasePartioFile(Partio::ParticlesInfo* file)
{
    if(file) file->release();
}


PointArrayModel::PointArrayModel()
    : m_npoints(0)
{ }


bool PointArrayModel::loadPointFile(const QString& fileName)
{
    namespace Pio = Partio;
    boost::shared_ptr<Pio::ParticlesData> ptFile(
                Pio::read(fileName.toAscii().constData()), releasePartioFile);
    if(!ptFile)
    {
        QMessageBox::critical(0, tr("Error"),
            tr("Couldn't open file \"%1\"").arg(fileName));
        return false;
    }
    m_fileName = fileName;
    // Look for the necessary attributes in the file
    Pio::ParticleAttribute positionAttr;
    Pio::ParticleAttribute normalAttr;
    Pio::ParticleAttribute radiusAttr;
    Pio::ParticleAttribute colorAttr;
    if(!ptFile->attributeInfo("position", positionAttr) ||
       !ptFile->attributeInfo("normal", normalAttr)   ||
       !ptFile->attributeInfo("radius", radiusAttr))
    {
        QMessageBox::critical(0, tr("Error"),
            tr("Couldn't find required attribute in \"%1\"").arg(fileName));
        return false;
    }
    // Check types
    if(positionAttr.type != Pio::VECTOR ||
       normalAttr.type != Pio::VECTOR ||
       radiusAttr.type != Pio::FLOAT || radiusAttr.count != 1)
    {
        QMessageBox::critical(0, tr("Error"),
            tr("Point attribute type or size wrong in \"%1\"").arg(fileName));
        return false;
    }
    // Allocate required attributes
    m_npoints = ptFile->numParticles();
    m_P.reset(new V3f[m_npoints]);
    m_N.reset(new V3f[m_npoints]);
    m_r.reset(new float[m_npoints]);
    // Look for likely color channels & allocate space if necessary
    m_colorChannelNames = findColorChannels(ptFile.get());
    m_color.reset();
    if(!m_colorChannelNames.empty())
    {
        if(ptFile->attributeInfo(m_colorChannelNames[0].toAscii().constData(), colorAttr))
            m_color.reset(new C3f[m_npoints]);
    }
    // Iterate over all particles & pull in the data.
    Pio::ParticleAccessor posAcc(positionAttr);
    Pio::ParticleAccessor norAcc(normalAttr);
    Pio::ParticleAccessor radiusAcc(radiusAttr);
    Pio::ParticleAccessor colorAcc(colorAttr);
    Pio::ParticlesData::const_iterator pt = ptFile->begin();
    pt.addAccessor(posAcc);
    pt.addAccessor(norAcc);
    pt.addAccessor(radiusAcc);
    if(m_color)
        pt.addAccessor(colorAcc);
    V3f* outP = m_P.get();
    V3f* outN = m_N.get();
    float* outr = m_r.get();
    C3f* outc = m_color.get();
    for(; pt != ptFile->end(); ++pt)
    {
        *outP++ = posAcc.data<V3f>(pt);
        *outN++ = norAcc.data<V3f>(pt);
        *outr++ = radiusAcc.data<float>(pt);
        if(m_color)
            *outc++ = colorAcc.data<C3f>(pt);
    }
    return true;
}


void PointArrayModel::setColorChannel(const QString& name)
{
    namespace Pio = Partio;
    boost::shared_ptr<Pio::ParticlesData> ptFile(
                Pio::read(m_fileName.toAscii().constData()), releasePartioFile);
    Pio::ParticleAttribute colorAttr;
    if(!ptFile->attributeInfo(name.toAscii().constData(), colorAttr)
       || colorAttr.count != 3 || colorAttr.type != Pio::FLOAT)
        return;
    m_color.reset(new C3f[m_npoints]);
    Pio::ParticleAccessor colorAcc(colorAttr);
    Pio::ParticlesData::const_iterator pt = ptFile->begin();
    pt.addAccessor(colorAcc);
    C3f* outc = m_color.get();
    for(; pt != ptFile->end(); ++pt, ++outc)
        *outc = colorAcc.data<C3f>(pt);
}


V3f PointArrayModel::centroid() const
{
    V3f sum(0);
    const V3f* P = m_P.get();
    for(size_t i = 0; i < m_npoints; ++i, ++P)
        sum += *P;
    return (1.0f/m_npoints) * sum;
}


QStringList PointArrayModel::findColorChannels(const Partio::ParticlesInfo* ptFile)
{
    QStringList colChans;
    Partio::ParticleAttribute attr;
    for(int i = 0; i < ptFile->numAttributes(); ++i)
    {
        ptFile->attributeInfo(i, attr);
        if(attr.type == Partio::FLOAT && attr.count == 3)
            colChans.push_back(QString::fromStdString(attr.name));
    }
    // TODO: Sort by some criterion of "most color-like" channels?
    return colChans;
}


//----------------------------------------------------------------------
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
#endif


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
    // Examine node bound and cull if possible
    float r = node->aggR;
    V3f p = node->aggP - P;
    float plen2 = p.length2();
    // Examine solid angle of interior node bounding sphere to see whether we
    // can render it directly or not.
    float solidAngle = M_PI*r*r / plen2;
    if(solidAngle < maxSolidAngle)
    {
        drawDisk(node->aggP, node->aggN, r);
    }
    else
    {
        // If the solid angle is too large consider child nodes or child
        // points.
        if(node->npoints != 0)
        {
            // Leaf node: simply render each child point.
            for(int i = 0; i < node->npoints; ++i)
            {
                const float* data = &node->data[i*dataSize];
                V3f p = V3f(data[0], data[1], data[2]);
                V3f n = V3f(data[3], data[4], data[5]);
                float r = data[6];
                drawDisk(p, n, r);
            }
            return;
        }
        else
        {
            // Interior node: render each non-null child.
            for(int i = 0; i < 8; ++i)
            {
                PointOctree::Node* child = node->children[i];
                if(!child)
                    continue;
                splitNode(P, maxSolidAngle, dataSize, child);
            }
        }
    }
}


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
    m_camera(true),
    m_lastPos(0,0),
    m_zooming(false),
    m_cursorPos(0),
    m_probeRes(10),
    m_probeMaxSolidAngle(0),
    m_backgroundColor(0, 0, 0),
    m_visMode(Vis_Points),
    m_drawAxes(false),
    m_lighting(false),
    m_points(),
    m_pointTree(),
    m_cloudCenter(0)
{
    setFocusPolicy(Qt::StrongFocus);

    connect(&m_camera, SIGNAL(projectionChanged()), this, SLOT(updateGL()));
    connect(&m_camera, SIGNAL(viewChanged()), this, SLOT(updateGL()));
}


void PointView::loadPointFiles(const QStringList& fileNames)
{
    m_points.clear();
    for(int i = 0; i < fileNames.size(); ++i)
    {
        boost::shared_ptr<PointArrayModel> points(new PointArrayModel());
        if(points->loadPointFile(fileNames[i]) && !points->empty())
            m_points.push_back(points);
    }
    if(m_points.empty())
        return;
    emit colorChannelsChanged(m_points[0]->colorChannels());
    m_cloudCenter = m_points[0]->centroid();
    m_cursorPos = m_cloudCenter;
    m_camera.setCenter(exr2qt(m_cloudCenter));
#if 0
    // Debug
    PointArray a;
    loadPointFile(a, fileNames[0].toStdString());
    m_pointTree.reset(); // free up memory
    m_pointTree = boost::shared_ptr<PointOctree>(new PointOctree(a));
#endif
    updateGL();
}


void PointView::setProbeParams(int cubeFaceRes, float maxSolidAngle)
{
    m_probeRes = cubeFaceRes;
    m_probeMaxSolidAngle = maxSolidAngle;
}


PointView::VisMode PointView::visMode() const
{
    return m_visMode;
}


void PointView::setBackground(QColor col)
{
    m_backgroundColor = col;
    updateGL();
}


void PointView::setVisMode(VisMode mode)
{
    m_visMode = mode;
    updateGL();
}


void PointView::toggleDrawAxes()
{
    m_drawAxes = !m_drawAxes;
}


void PointView::setColorChannel(QString channel)
{
    for(size_t i = 0; i < m_points.size(); ++i)
        m_points[i]->setColorChannel(channel);
    updateGL();
}


QSize PointView::sizeHint() const
{
    // Size hint, mainly for getting the initial window size right.
    // setMinimumSize() also sort of works for this, but doesn't allow the
    // user to later make the window smaller.
    return QSize(640,480);
}


void PointView::initializeGL()
{
    //glEnable(GL_MULTISAMPLE);
}


void PointView::resizeGL(int w, int h)
{
    // Draw on full window
    glViewport(0, 0, w, h);
    m_camera.setViewport(geometry());
}


void PointView::paintGL()
{
    //--------------------------------------------------
    // Draw main scene
    // Set camera projection
    glMatrixMode(GL_PROJECTION);
    glLoadMatrix(qt2exr(m_camera.projectionMatrix()));
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrix(qt2exr(m_camera.viewMatrix()));

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(m_backgroundColor.redF(), m_backgroundColor.greenF(),
                 m_backgroundColor.blueF(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw geometry

    if(m_drawAxes)
        drawAxes();
    for(size_t i = 0; i < m_points.size(); ++i)
        drawPoints(*m_points[i], m_visMode, m_lighting);
//    if(m_pointTree)
//        splitNode(m_cursorPos, m_probeMaxSolidAngle, m_pointTree->dataSize(),
//                  m_pointTree->root());


    if(m_pointTree)
    {
        RadiosityIntegrator integrator(m_probeRes);
        // Get a vector toward the probe position from the camera center
        QMatrix4x4 viewMatrix = m_camera.viewMatrix();
        V3f d = (qt2exr(viewMatrix.inverted().
                        map(viewMatrix.map(exr2qt(m_cursorPos))*1.1)) -
                 m_cursorPos).normalized();
        float coneAngle = M_PI;
        microRasterize(integrator, m_cursorPos, d,
                       coneAngle,
                       m_probeMaxSolidAngle, *m_pointTree);
        // Draw image of rendered microbuffer
        glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT | GL_ENABLE_BIT);
        glPushMatrix();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_SCISSOR_TEST);

        GLuint microBufWidth = width()/3;
        glScissor(width() - microBufWidth, 0,
                  microBufWidth, microBufWidth*3/4);
        glViewport(width() - microBufWidth, 0,
                   microBufWidth, microBufWidth*3/4);
        drawMicroBuf(integrator.microBuf());

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopAttrib();
    }

    // Draw overlay stuff, including cursor position.
    drawCursor(m_cursorPos);
}


void PointView::mousePressEvent(QMouseEvent* event)
{
    m_zooming = event->button() == Qt::RightButton;
    m_lastPos = event->pos();
}


void PointView::mouseMoveEvent(QMouseEvent* event)
{
    if(event->modifiers() & Qt::ControlModifier)
    {
        m_cursorPos = qt2exr(
            m_camera.mouseMovePoint(exr2qt(m_cursorPos),
                                    event->pos() - m_lastPos,
                                    m_zooming) );
        updateGL();
    }
    else
        m_camera.mouseDrag(m_lastPos, event->pos(), m_zooming);
    m_lastPos = event->pos();
}


void PointView::wheelEvent(QWheelEvent* event)
{
    // Translate mouse wheel events into vertical dragging for simplicity.
    m_camera.mouseDrag(QPoint(0,0), QPoint(0, -event->delta()/2), true);
}


void PointView::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_L)
    {
        m_lighting = !m_lighting;
        updateGL();
    }
    else if(event->key() == Qt::Key_C)
    {
        m_camera.setCenter(exr2qt(m_cursorPos));
    }
    else if(event->key() == Qt::Key_S)
    {
        // Snap probe to position of closest point and center on it
        V3f newPos(0);
        float nearestDist = FLT_MAX;
        for(size_t j = 0; j < m_points.size(); ++j)
        {
            if(m_points[j]->empty())
                continue;
            const V3f* P = m_points[j]->P();
            for(size_t i = 0, iend = m_points[j]->size(); i < iend; ++i, ++P)
            {
                float dist = (m_cursorPos - *P).length2();
                if(dist < nearestDist)
                {
                    nearestDist = dist;
                    newPos = *P;
                }
            }
        }
        m_cursorPos = newPos;
        m_camera.setCenter(exr2qt(newPos));
        updateGL();
    }
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


/// Draw the 3D cursor
void PointView::drawCursor(const V3f& p) const
{
    // Draw a point at the centre of the cursor.
    glColor3f(1,1,1);
    glPointSize(10);
    glBegin(GL_POINTS);
        glVertex(m_cursorPos);
    glEnd();

    // Find position of cursor in screen space
    V3f screenP3 = qt2exr(m_camera.projectionMatrix()*m_camera.viewMatrix() *
                          exr2qt(m_cursorPos));
    if(screenP3.z < 0)
        return; // Cull if behind the camera.  TODO: doesn't work quite right

    // Now draw a 2D overlay over the 3D scene to allow user to pinpoint the
    // cursor, even when when it's behind something.
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0,width(), 0,height(), 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    // Position in ortho coord system
    V2f p2 = 0.5f * V2f(width(), height()) *
             (V2f(screenP3.x, screenP3.y) + V2f(1.0f));
    float r = 10;
    glLineWidth(2);
    glColor3f(1,1,1);
    // Combined white and black crosshairs, so they can be seen on any
    // background.
    glTranslatef(p2.x, p2.y, 0);
    glBegin(GL_LINES);
        glVertex(V2f(r,   0));
        glVertex(V2f(2*r, 0));
        glVertex(-V2f(r,   0));
        glVertex(-V2f(2*r, 0));
        glVertex(V2f(0,   r));
        glVertex(V2f(0, 2*r));
        glVertex(-V2f(0,   r));
        glVertex(-V2f(0, 2*r));
    glEnd();
    glColor3f(0,0,0);
    glRotatef(45,0,0,1);
    glBegin(GL_LINES);
        glVertex(V2f(r,   0));
        glVertex(V2f(2*r, 0));
        glVertex(-V2f(r,   0));
        glVertex(-V2f(2*r, 0));
        glVertex(V2f(0,   r));
        glVertex(V2f(0, 2*r));
        glVertex(-V2f(0,   r));
        glVertex(-V2f(0, 2*r));
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib();
}


/// Draw point cloud using OpenGL
void PointView::drawPoints(const PointArrayModel& points, VisMode visMode,
                           bool useLighting)
{
    if(points.empty())
        return;
    switch(visMode)
    {
        case Vis_Points:
        {
            glDisable(GL_COLOR_MATERIAL);
            glDisable(GL_LIGHTING);
            // Draw points
            glPointSize(1);
            glColor3f(1,1,1);
            // Set distance attenuation for points, following the usual 1/z
            // law.
            //GLfloat attenParams[3] = {0, 0, 1};
            //glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, attenParams);
            //glPointParameterf(GL_POINT_SIZE_MIN, 0);
            //glPointParameterf(GL_POINT_SIZE_MAX, 100);
            // Draw all points at once using vertex arrays.
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, 3*sizeof(float),
                            reinterpret_cast<const float*>(points.P()));
            const float* col = reinterpret_cast<const float*>(points.color());
            if(col)
            {
                glEnableClientState(GL_COLOR_ARRAY);
                glColorPointer(3, GL_FLOAT, 3*sizeof(float), col);
            }
            glDrawArrays(GL_POINTS, 0, points.size());
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
        }
        break;
        case Vis_Disks:
        {
            // Materials
            glShadeModel(GL_SMOOTH);
            if(useLighting)
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
                //glEnable(GL_RESCALE_NORMAL);
            }
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
            const V3f* P = points.P();
            const V3f* N = points.N();
            const float* r = points.r();
            const C3f* col = points.color();
            for(size_t i = 0; i < points.size(); ++i, ++P, ++N, ++r)
            {
                glColor(col ? *col++ : C3f(1));
                if(N->length2() == 0)
                {
                    // For zero-length normals, we don't know the disk
                    // orientation, so draw as a point instead.
                    glPointSize(1);
                    glBegin(GL_POINTS);
                        glVertex(*P);
                    glEnd();
                    continue;
                }
                glPushMatrix();
                // Translate disk to point location and scale
                glTranslate(*P);
                glScalef(*r, *r, *r);
                // Transform the disk normal (0,0,1) into the correct normal
                // direction for the current point.  The appropriate transform
                // is a rotation about a direction perpendicular to both
                // normals,
                V3f v = diskNormal % *N;
                if(v.length2() > 1e-10)
                {
                    // And via the angle given by the dot product.  (If the
                    // length of v is very small we don't do the rotation for
                    // numerical stability.)
                    float angle = rad2deg(acosf(diskNormal.dot(*N)));
                    glRotatef(angle, v.x, v.y, v.z);
                }
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


//------------------------------------------------------------------------------
// PointViewerMainWindow implementation

PointViewerMainWindow::PointViewerMainWindow(
        const QStringList& initialPointFileNames)
    : m_pointView(0),
    m_colorMenu(0),
    m_colorMenuGroup(0),
    m_colorMenuMapper(0)
{
    setWindowTitle("Aqsis point cloud viewer");

    // File menu
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    QAction* openAct = fileMenu->addAction(tr("&Open"));
    openAct->setStatusTip(tr("Open a point cloud file"));
    openAct->setShortcuts(QKeySequence::Open);
    connect(openAct, SIGNAL(triggered()), this, SLOT(openFiles()));
    QAction* quitAct = fileMenu->addAction(tr("&Quit"));
    quitAct->setStatusTip(tr("Exit the application"));
    quitAct->setShortcuts(QKeySequence::Quit);
    connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));

    // View menu
    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    QAction* viewAsDisks = viewMenu->addAction(tr("Draw As &Disks"));
    viewAsDisks->setShortcut(tr("v"));
    viewAsDisks->setCheckable(true);
    connect(viewAsDisks, SIGNAL(triggered()), this, SLOT(toggleVisMode()));
    QAction* drawAxes = viewMenu->addAction(tr("Draw &Axes"));
    drawAxes->setCheckable(true);
    // Background sub-menu
    QMenu* backMenu = viewMenu->addMenu(tr("Set &Background"));
    QSignalMapper* mapper = new QSignalMapper(this);
    // Selectable backgrounds (svg_names from SVG standard - see QColor docs)
    const char* backgroundNames[] = {/* "Display Name", "svg_name", */
                                        "&Black",        "black",
                                        "&Dark Grey",    "dimgrey",
                                        "&Light Grey",   "lightgrey",
                                        "&White",        "white" };
    for(size_t i = 0; i < sizeof(backgroundNames)/sizeof(const char*); i+=2)
    {
        QAction* backgroundAct = backMenu->addAction(tr(backgroundNames[i]));
        mapper->setMapping(backgroundAct, backgroundNames[i+1]);
        connect(backgroundAct, SIGNAL(triggered()), mapper, SLOT(map()));
    }
    connect(mapper, SIGNAL(mapped(QString)),
            this, SLOT(setBackground(QString)));
    backMenu->addSeparator();
    QAction* backgroundCustom = backMenu->addAction(tr("&Custom"));
    connect(backgroundCustom, SIGNAL(triggered()),
            this, SLOT(chooseBackground()));
    // Color channel menu
    m_colorMenu = viewMenu->addMenu(tr("Color &Channel"));
    m_colorMenuMapper = new QSignalMapper(this);

    // Help menu
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction* helpAct = helpMenu->addAction(tr("&Controls"));
    connect(helpAct, SIGNAL(triggered()), this, SLOT(helpDialog()));
    helpMenu->addSeparator();
    QAction* aboutAct = helpMenu->addAction(tr("&About"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(aboutDialog()));

    m_pointView = new PointView(this);
    connect(m_pointView, SIGNAL(colorChannelsChanged(QStringList)),
            this, SLOT(setColorChannels(QStringList)));
    connect(m_colorMenuMapper, SIGNAL(mapped(QString)),
            m_pointView, SLOT(setColorChannel(QString)));
    connect(drawAxes, SIGNAL(triggered()),
            m_pointView, SLOT(toggleDrawAxes()));

    setCentralWidget(m_pointView);
    if(!initialPointFileNames.empty())
        m_pointView->loadPointFiles(initialPointFileNames);
}


void PointViewerMainWindow::keyReleaseEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_Escape)
        close();
}


void PointViewerMainWindow::openFiles()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
            tr("Select one or more point clouds to open"), "",
            tr("Point cloud files (*.ptc)"));
    if(!fileNames.empty())
        m_pointView->loadPointFiles(fileNames);
}


void PointViewerMainWindow::helpDialog()
{
    QString message = tr(
        "ptview controls:\n"
        "\n"
        "LMB+drag = rotate camera\n"
        "RMB+drag = zoom camera\n"
        "Ctrl+LMB+drag = move 3D cursor\n"
        "Ctrl+RMB+drag = zoom 3D cursor along view direction\n"
        "'c' = center camera on 3D cursor\n"
        "'v' = toggle view mode between points and disks\n"
        "'s' = snap 3D cursor to nearest point\n"
        "\n"
        "(LMB, RMB = left & right mouse buttons)\n"
    );
    QMessageBox::information(this, tr("ptview control summary"), message);
}


void PointViewerMainWindow::aboutDialog()
{
    QString message = tr("Aqsis point cloud viewer\nversion %1.")
                      .arg(AQSIS_VERSION_STR_FULL);
    QMessageBox::information(this, tr("About ptview"), message);
}


void PointViewerMainWindow::setBackground(const QString& name)
{
    m_pointView->setBackground(QColor(name));
}


void PointViewerMainWindow::chooseBackground()
{
    m_pointView->setBackground(
        QColorDialog::getColor(QColor(255,255,255), this, "background color"));
}


void PointViewerMainWindow::toggleVisMode()
{
    m_pointView->setVisMode(m_pointView->visMode() == PointView::Vis_Points ?
                            PointView::Vis_Disks : PointView::Vis_Points);
}


void PointViewerMainWindow::setColorChannels(QStringList channels)
{
    // Remove the old set of color channels from the menu
    delete m_colorMenuGroup;
    m_colorMenuGroup = new QActionGroup(this);
    m_colorMenuGroup->setExclusive(true);
    if(channels.empty())
        return;
    // Rebuild the color channel menu with a menu item for each channel
    for(int i = 0; i < channels.size(); ++i)
    {
        QAction* act = m_colorMenuGroup->addAction(channels[i]);
        act->setCheckable(true);
        m_colorMenu->addAction(act);
        m_colorMenuMapper->setMapping(act, channels[i]);
        connect(act, SIGNAL(triggered()), m_colorMenuMapper, SLOT(map()));
    }
    m_colorMenuGroup->actions()[0]->setChecked(true);
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
        ("maxsolidangle,a", po::value<float>()->default_value(0.02),
         "max solid angle used for aggreates in point hierarchy during rendering")
        ("proberes,p", po::value<int>()->default_value(10),
         "resolution of micro environment raster for viewing")
        ("cloudres,c", po::value<float>()->default_value(20),
         "resolution of point cloud")
        ("radiusmult,r", po::value<float>()->default_value(1),
         "multiplying factor for surfel radius")
        ("point_files", po::value<std::vector<std::string> >()->default_value(std::vector<std::string>(), "[]"),
         "file to display")
    ;
    // Parse options
    po::variables_map opts;
    po::positional_options_description positionalOpts;
    positionalOpts.add("point_files", -1);
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

    // Convert std::vector<std::string> into QStringList...
    const std::vector<std::string>& pointFileNamesStd =
                        opts["point_files"].as<std::vector<std::string> >();
    QStringList pointFileNames;
    for(int i = 0, iend = pointFileNamesStd.size(); i < iend; ++i)
        pointFileNames.push_back(QString::fromStdString(pointFileNamesStd[i]));

    PointViewerMainWindow window(pointFileNames);
    float maxSolidAngle = opts["maxsolidangle"].as<float>();
    int probeRes = opts["proberes"].as<int>();
    window.pointView().setProbeParams(probeRes, maxSolidAngle);
    window.show();

    return app.exec();
}


// vi: set et:
