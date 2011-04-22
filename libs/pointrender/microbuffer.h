#include "pointcloud.h"

#include <cfloat>
#include <cmath>

#include <boost/scoped_array.hpp>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathMath.h>

#ifndef AQSIS_MICROBUFFER_H_INCLUDED
#define AQSIS_MICROBUFFER_H_INCLUDED

using Imath::V3f;

inline float dot(V3f a, V3f b)
{
    return a^b;
}

/// An axis-aligned cube environment buffer.
///
/// Just stores depth (z) for now (TODO)
///
/// Each face has a coordinate system where the centres of the boundary pixels
/// lie just _inside_ the boundary.  That is, the positions of the pixel point
/// samples are at x_i = (1/2 + i/N) for i = 0 to N-1.
///
/// For example, with 3x3 pixel faces, each
/// face looks like
///
///   +-----------+
///   | x   x   x |
///   |           |
///   | x   x   x |
///   |           |
///   | x   x   x |
///   +-----------+
///
/// where the x's represent the positions at which point sampling will occur.
///
/// The orientation of the faces is
///
/// TODO
///
class MicroBuf
{
    public:
        /// Identifiers for each cube face direction
        enum Face
        {
            Face_xp, ///< x+
            Face_yp, ///< y+
            Face_zp, ///< z+
            Face_xn, ///< x-
            Face_yn, ///< y-
            Face_zn, ///< z-
            Face_end,
            Face_begin = Face_xp
        };

        MicroBuf(int faceRes)
            : m_res(faceRes),
            m_faceSize(faceRes*faceRes),
            m_pixels()
        {
            m_pixels.reset(new float[faceRes*faceRes*Face_end]);
            reset();
        }

        /// Reset buffer to default (non-rendered) state.
        void reset()
        {
            for(int i = 0, iend = size(); i < iend; ++i)
                m_pixels[i] = FLT_MAX;
        }

        /// Get raw data store for face
        float* face(int which)
        {
            assert(which >= Face_begin && which < Face_end);
            return &m_pixels[0] + which*m_faceSize;
        }
        const float* face(int which) const
        {
            assert(which >= Face_begin && which < Face_end);
            return &m_pixels[0] + which*m_faceSize;
        }

        // Get direction vector for position on a given face.
        V3f direction(int whichFace, int iu, int iv) const
        {
            // TODO: Cache these coefficients to get (iu*scale + offset);
            float u = (0.5f + iu)/m_res*2.0f - 1.0f;
            float v = (0.5f + iv)/m_res*2.0f - 1.0f;
            switch(whichFace)
            {
                case Face_xp: return V3f( 1, v,-u).normalized();
                case Face_yp: return V3f( u, 1,-v).normalized();
                case Face_zp: return V3f( u, v, 1).normalized();
                case Face_xn: return V3f(-1, v, u).normalized();
                case Face_yn: return V3f( u,-1, v).normalized();
                case Face_zn: return V3f(-u, v,-1).normalized();
                default: assert(0 && "unknown face"); return V3f();
            }
        }

        /// Face side resolution
        int res() const { return m_res; }
        /// Total size of all faces in number of texels
        int size() const { return Face_end*m_res*m_res; }

    private:
        // Square faces
        int m_res;
        // Number of floats needed to store a face
        int m_faceSize;
        // Pixel face storage order:  +x -x +y -y +z -z
        boost::scoped_array<float> m_pixels;
};


/// Render points into a micro environment buffer.
///
/// \param microBuf - destination environment buffer
/// \param P - position of light probe
/// \param N - normal for light probe (should be normalized)
/// \param dotCut - defines cone angle of interest about N
///                 (cos(angle) = dotCut).
/// \param points - point cloud to render
void microRasterize(MicroBuf& microBuf, V3f P, V3f N, float dotCut,
                    const PointArray& points)
{
    int faceRes = microBuf.res();
    float rasterScale = 0.5f*microBuf.res();
    for(int pIdx = 0, npoints = points.size(); pIdx < npoints; ++pIdx)
    {
        const float* data = &points.data[pIdx*points.stride];
        // normal of current point
        V3f n = V3f(data[3], data[4], data[5]);
        // position of current point relative to shading point
        V3f p = V3f(data[0], data[1], data[2]) - P;
        float plen = p.length();
        // TODO: what about disks sticking out from just below the horizon?
        if(dot(p, N) < plen*dotCut)
            continue;
        float u = 0;
        float v = 0;
        // Figure out which face we're on and get u,v coordinates on that face,
        // along with a pointer to the face data.
        // The coordinates have the range -1 <= u,v <= 1.
        V3f absp = V3f(fabs(p.x), fabs(p.y), fabs(p.z));
        MicroBuf::Face faceIndex = MicroBuf::Face_xp;
        if(absp.x >= absp.y && absp.x >= absp.z) // x faces
        {
            u = -p.z/p.x;
            v = p.y/absp.x;
            faceIndex = (p.x > 0) ? MicroBuf::Face_xp : MicroBuf::Face_xn;
        }
        else if(absp.y >= absp.x && absp.y >= absp.z) // y faces
        {
            u = p.x/absp.y;
            v = -p.z/p.y;
            faceIndex = (p.y > 0) ? MicroBuf::Face_yp : MicroBuf::Face_yn;
        }
        else // z faces
        {
            assert(absp.z >= absp.x && absp.z >= absp.y);
            u = p.x/p.z;
            v = p.y/absp.z;
            faceIndex = (p.z > 0) ? MicroBuf::Face_zp : MicroBuf::Face_zn;
        }
        float* face = microBuf.face(faceIndex);
        // Compute the area of the surfel when projected onto the env face:
        // The disk radius
        float r = data[6];
        float origArea = M_PI*r*r;
        // The angle between the disk normal and viewing vector (note that
        // p/plen is normalized)
        float angleFactor = fabs(dot(p, n)/plen);
        // Distance to the surfel vs distance to face
        float faceDist2 = 1 + u*u + v*v;
        float distFactor = faceDist2/(plen*plen);
        // Projected area
        float projArea = origArea * angleFactor * distFactor;
        // Half-width of a square with area projArea:
        // FIXME: Remove factor of 2.
        float wOn2 = 2* sqrt(projArea)*0.5f;
        // TODO: detect overlap onto other faces:  if(u-wOn2 < -1) etc
        // Next, transform to face raster coords.
        u = rasterScale*(u + 1.0f);
        v = rasterScale*(v + 1.0f);
        wOn2 *= rasterScale;
        // Construct square box with the correct area.  This shape isn't
        // anything like the true projection of a disk onto the raster, but
        // it's much cheaper!  Note that points which are proxies for clusters
        // of smaller points aren't going to be accurately resolved no matter
        // what we do.
        int ustart = Imath::clamp(int(u - wOn2 + 0.5f), 0, faceRes);
        int uend   = Imath::clamp(int(u + wOn2 + 0.5f), 0, faceRes);
        int vstart = Imath::clamp(int(v - wOn2 + 0.5f), 0, faceRes);
        int vend   = Imath::clamp(int(v + wOn2 + 0.5f), 0, faceRes);
        for(int iv = vstart; iv < vend; ++iv)
        for(int iu = ustart; iu < uend; ++iu)
        {
            int pixelIndex = iv*faceRes + iu;
            float d = face[pixelIndex];
            if(plen < d)
                face[pixelIndex] = plen;
        }
    }
}


/// Compute ambient occlusion based on depths held in a micro env buffer.
///
/// This is one minus the zero-bounce light coming from infinity to the point.
///
/// \param depthBuf - environment buffer of depths at a point
/// \param N - normal at point
float occlusion(const MicroBuf& depthBuf, const V3f& N)
{
    float illum = 0;
    float totWeight = 0;
    for(int f = MicroBuf::Face_begin; f < MicroBuf::Face_end; ++f)
    {
        const float* depth = depthBuf.face(f);
        for(int iv = 0; iv < depthBuf.res(); ++iv)
        for(int iu = 0; iu < depthBuf.res(); ++iu, ++depth)
        {
            float d = dot(depthBuf.direction(f, iu, iv), N);
            if(d > 0)
            {
                // Accumulate light coming from infinity.
                if(depth[0] == FLT_MAX)
                    illum += d;
                totWeight += d;
            }
        }
    }
    illum /= totWeight;
    return 1 - illum;
}

#if 0
void getDirection(MicroBuf& depthBuf, int comp)
{
    for(int f = MicroBuf::Face_begin; f < MicroBuf::Face_end; ++f)
    {
        float* z = depthBuf.face(f);
        for(int iv = 0; iv < depthBuf.res(); ++iv)
        for(int iu = 0; iu < depthBuf.res(); ++iu, ++z)
        {
            *z = depthBuf.direction(f, iu, iv)[comp];
        }
    }
}
#endif


/// Compute total radient energy at a position
///
/// (Fluence is the radient energy integrated over all solid angles.)
float radiantFluence(const MicroBuf& depthBuf)
{
    float illum = 0;
    for(int f = MicroBuf::Face_begin; f < MicroBuf::Face_end; ++f)
    {
        const float* depth = depthBuf.face(f);
        for(int iv = 0; iv < depthBuf.res(); ++iv)
        for(int iu = 0; iu < depthBuf.res(); ++iu, ++depth)
        {
            // Accumulate light coming from infinity.
            if(depth[0] == FLT_MAX)
                ++illum;
        }
    }
    illum /= depthBuf.size();
    return illum;
}


/// Bake occlusion from point array back into point array.
void bakeOcclusion(PointArray& points)
{
    const int faceRes = 20;
    float eps = 0.02;
    MicroBuf depthBuf(faceRes);
    for(int pIdx = 0, npoints = points.size(); pIdx < npoints; ++pIdx)
    {
        std::cout << 100.0f*pIdx/points.size() << "%\n";
        float* data = &points.data[pIdx*points.stride];
        // normal of current point
        V3f N = V3f(data[3], data[4], data[5]);
        // position of current point relative to shading point
        V3f P = V3f(data[0], data[1], data[2]);
        depthBuf.reset();
        microRasterize(depthBuf, P + N*eps, N, 0, points);
        float occl = occlusion(depthBuf, N);
        data[7] = data[8] = data[9] = 1 - occl;
    }
}


#endif // AQSIS_MICROBUFFER_H_INCLUDED

// vi: set et:
