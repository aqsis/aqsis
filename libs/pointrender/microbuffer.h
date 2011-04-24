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
///              +---+
///   ^          |+y |
///   |  +---+---+---+---+
///  v|  |-z |-x |+z |+x |
///   |  +---+---+---+---+
///   |          |-y |
///   |          +---+
///   |     u
///   +-------->
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

        MicroBuf(int faceRes, int nchans = 1)
            : m_res(faceRes),
            m_nchans(nchans),
            m_faceSize(nchans*faceRes*faceRes),
            m_pixels()
        {
            m_pixels.reset(new float[m_faceSize*Face_end]);
            reset();
        }

        /// Reset buffer to default (non-rendered) state.
        ///
        /// TODO: Reset for non-z data.
        void reset()
        {
            for(int i = 0, iend = size(); i < iend; ++i)
            {
                m_pixels[2*i] = FLT_MAX;
                m_pixels[2*i + 1] = 0;
            }
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

        /// Get index of face which direction p sits inside.
        static Face faceIndex(V3f p)
        {
            V3f absp = V3f(fabs(p.x), fabs(p.y), fabs(p.z));
            if(absp.x >= absp.y && absp.x >= absp.z)
                return (p.x > 0) ? MicroBuf::Face_xp : MicroBuf::Face_xn;
            else if(absp.y >= absp.x && absp.y >= absp.z)
                return (p.y > 0) ? MicroBuf::Face_yp : MicroBuf::Face_yn;
            else
            {
                assert(absp.z >= absp.x && absp.z >= absp.y);
                return (p.z > 0) ? MicroBuf::Face_zp : MicroBuf::Face_zn;
            }
        }

        /// Get a neighbouring face in u direction
        ///
        /// \param faceIdx - current face index
        /// \param side - which side to look (0 == left, 1 == right)
        ///
        // +---+---+---+  +---+---+---+  +---+---+---+
        // |+z |+x |-z |  |-x |+y |+x |  |-x |+z |+x |
        // +---+---+---+  +---+---+---+  +---+---+---+
        //
        // +---+---+---+  +---+---+---+  +---+---+---+
        // |-z |-x |+z |  |-x |-y |+x |  |+x |-z |-x |
        // +---+---+---+  +---+---+---+  +---+---+---+
        //
        static Face neighbourU(int faceIdx, int side)
        {
            static Face neighbourArray[6][2] = {
                {Face_zp, Face_zn}, {Face_xn, Face_xp}, {Face_xn, Face_xp},
                {Face_zn, Face_zp}, {Face_xn, Face_xp}, {Face_xp, Face_xn}
            };
            return neighbourArray[faceIdx][side];
        }

        /// Get a neighbouring face in v direction
        ///
        /// \param faceIdx - current face index
        /// \param side - which side to look (0 == bottom, 1 == top)
        ///
        // +---+   +---+   +---+   +---+   +---+   +---+
        // |+y |   |-z |   |+y |   |+y |   |+z |   |+y |
        // +---+   +---+   +---+   +---+   +---+   +---+
        // |+x |   |+y |   |+z |   |-x |   |-y |   |-z |
        // +---+   +---+   +---+   +---+   +---+   +---+
        // |-y |   |+z |   |-y |   |-y |   |-z |   |-y |
        // +---+   +---+   +---+   +---+   +---+   +---+
        static Face neighbourV(int faceIdx, int side)
        {
            static Face neighbourArray[6][2] = {
                {Face_yn, Face_yp}, {Face_zp, Face_zn}, {Face_yn, Face_yp},
                {Face_yn, Face_yp}, {Face_zn, Face_zp}, {Face_yn, Face_yp}
            };
            return neighbourArray[faceIdx][side];
        }

        /// Get coordinates on face
        ///
        /// The coordinates are in the range -1 <= u,v <= 1, if faceIdx is
        /// obtained using the faceIndex function.  Coordinates outside this
        /// range are legal, as long as p has nonzero component in the
        /// direction of the normal of the face.
        ///
        /// \param faceIdx - index of current face
        /// \param p - position (may lie outside cone of current face)
        static void faceCoords(int faceIdx, V3f p, float& u, float& v)
        {
            switch(faceIdx)
            {
                case Face_xp: u = -p.z/p.x;  v =  p.y/p.x; break;
                case Face_xn: u = -p.z/p.x;  v = -p.y/p.x; break;
                case Face_yp: u =  p.x/p.y;  v = -p.z/p.y; break;
                case Face_yn: u = -p.x/p.y;  v = -p.z/p.y; break;
                case Face_zp: u =  p.x/p.z;  v =  p.y/p.z; break;
                case Face_zn: u =  p.x/p.z;  v = -p.y/p.z; break;
                default:
                    assert(0 && "invalid face");
                    break;
            }
        }

        /// Get direction vector for position on a given face.
        ///
        /// Roughly speaking, this is the opposite of the faceCoords function
        static V3f direction(int faceIdx, float u, float v)
        {
            switch(faceIdx)
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
        /// Number of channels per pixel
        int nchans() const { return m_nchans; }
        /// Total size of all faces in number of texels
        int size() const { return Face_end*m_res*m_res; }

    private:
        // Square faces
        int m_res;
        /// Number of channels per pixel
        int m_nchans;
        // Number of floats needed to store a face
        int m_faceSize;
        // Pixel face storage order:  +x -x +y -y +z -z
        boost::scoped_array<float> m_pixels;
};


/// Render a disk into the microbuffer using raytracing
///
/// When two surfaces meet at a sharp angle, the visibility of surfels on
/// the adjacent surface needs to be computed more carefully than with the
/// approximate rasterization method.  If not, disturbing artifacts will appear
/// where the surfaces join.  This function solves the problem by resolving
/// visibility of a disk using ray tracing.
///
/// \param p - position of disk center relative to microbuffer
/// \param n - disk normal
/// \param r - disk radius
void raytraceDisk(MicroBuf& microBuf, V3f p, V3f n, float r)
{
    int faceRes = microBuf.res();
    float plength = p.length();
    // Raytrace the disk if it's "close" as compared to the disk
    // radius.
    for(int iface = 0; iface < 6; ++iface)
    {
        float faceNormalDotP = p[iface%3]/plength *
                                ((iface < 3) ? 1.0f : -1.0f);
        // Cull face if we know the bounding cone of the face lies outside the
        // bounding cone of the disk.  TODO: More efficient raster bounding!
        if(faceNormalDotP < -1.0f/sqrt(3.0f))
            continue;
        float* face = microBuf.face(iface);
        for(int iv = 0; iv < faceRes; ++iv)
        for(int iu = 0; iu < faceRes; ++iu)
        {
            // d = ray through the pixel.
            V3f d = MicroBuf::direction(iface,
                            (0.5f + iu)/faceRes*2.0f - 1.0f,
                            (0.5f + iv)/faceRes*2.0f - 1.0f);
            // Intersect ray with plane containing disk.
            float t = dot(p, n)/dot(d, n);
            // Expand the disk just a little, to make the cracks a bit smaller.
            // We can't do this too much, or sharp convex edges will be
            // overoccluded (TODO: Adjust for best results!  Maybe the "too
            // large" problem could be worked around using a tracing offset?)
            const float extraRadius2 = 1.5f;
            if(t > 0 && (t*d - p).length2() < extraRadius2*r*r)
            {
                // The ray hit the disk, record the hit.
                int pixelIndex = 2*(iv*faceRes + iu);
                face[pixelIndex+1] = 1;
            }
        }
    }
}


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
        float r = data[6];
        // FIXME: should be 8*r (or so) here, but adjusted down for the time
        // being until efficiency is improved.
        if(plen < 2*r)
        {
            // Resolve visibility of very close surfels using ray tracing.
            // This is necessary to avoid artifacts where surfaces meet.
            raytraceDisk(microBuf, p, n, r);
            continue;
        }
        // TODO: what about disks sticking out from just below the horizon?
        if(dot(p, N) < plen*dotCut)
            continue;
        // Figure out which face we're on and get u,v coordinates on that face,
        MicroBuf::Face faceIndex = MicroBuf::faceIndex(p);
        float u = 0, v = 0;
        MicroBuf::faceCoords(faceIndex, p, u, v);
        // Compute the area of the surfel when projected onto the env face.
        // This depends on several things:
        // 1) The original disk
        float origArea = M_PI*r*r;
        // 2) The angles between the disk normal n, viewing vector p, and face
        // normal.  This is the viewed area before projection.
        float pDotFaceN = p[faceIndex % 3];
        float angleFactor = fabs(dot(p, n)/pDotFaceN);
        // 3) Ratio of distance to the surfel vs distance to projected point
        // on the face.
        float distFactor = 1.0f/(pDotFaceN*pDotFaceN);
        // Putting these together gives the projected area
        float projArea = origArea * angleFactor * distFactor;
//        float solidAngle = origArea * fabs(dot(p,n)) * distFactor;
        // Half-width of a square with area projArea
        float wOn2 = sqrt(projArea)*0.5f;
        // Transform width and position to face raster coords.
        u = rasterScale*(u + 1.0f);
        v = rasterScale*(v + 1.0f);
        wOn2 *= rasterScale;
        // Construct square box with the correct area.  This shape isn't
        // anything like the true projection of a disk onto the raster, but
        // it's much cheaper!  Note that points which are proxies for clusters
        // of smaller points aren't going to be accurately resolved no matter
        // what we do.
        struct BoundData
        {
            MicroBuf::Face faceIndex;
            float ubegin, uend;
            float vbegin, vend;
        };
        // The current surfel can cross up to three faces.
        int nfaces = 1;
        BoundData boundData[3];
        BoundData& bd0 = boundData[0];
        bd0.faceIndex = faceIndex;
        bd0.ubegin = u - wOn2;   bd0.uend = u + wOn2;
        bd0.vbegin = v - wOn2;   bd0.vend = v + wOn2;
        // Detect & handle overlap onto adjacent faces
        //
        // We assume that wOn2 is the same on the adjacent face, an assumption
        // which is true when the surfel is close the the corner of the cube.
        // We also assume that a surfel touches at most three faces.  This is
        // true as long as the surfels don't have a massive solid angle; for
        // such cases the axis-aligned box isn't going to be accurate anyway.
        if(bd0.ubegin < 0)
        {
            // left neighbour
            BoundData& b = boundData[nfaces++];
            b.faceIndex = MicroBuf::neighbourU(faceIndex, 0);
            MicroBuf::faceCoords(b.faceIndex, p, u, v);
            u = rasterScale*(u + 1.0f);
            v = rasterScale*(v + 1.0f);
            b.ubegin = u - wOn2;  b.uend = u + wOn2;
            b.vbegin = v - wOn2;  b.vend = v + wOn2;
        }
        else if(bd0.uend > faceRes)
        {
            // right neighbour
            BoundData& b = boundData[nfaces++];
            b.faceIndex = MicroBuf::neighbourU(faceIndex, 1);
            MicroBuf::faceCoords(b.faceIndex, p, u, v);
            u = rasterScale*(u + 1.0f);
            v = rasterScale*(v + 1.0f);
            b.ubegin = u - wOn2;  b.uend = u + wOn2;
            b.vbegin = v - wOn2;  b.vend = v + wOn2;
        }
        if(bd0.vbegin < 0)
        {
            // bottom neighbour
            BoundData& b = boundData[nfaces++];
            b.faceIndex = MicroBuf::neighbourV(faceIndex, 0);
            MicroBuf::faceCoords(b.faceIndex, p, u, v);
            u = rasterScale*(u + 1.0f);
            v = rasterScale*(v + 1.0f);
            b.ubegin = u - wOn2;  b.uend = u + wOn2;
            b.vbegin = v - wOn2;  b.vend = v + wOn2;
        }
        else if(bd0.vend > faceRes)
        {
            // top neighbour
            BoundData& b = boundData[nfaces++];
            b.faceIndex = MicroBuf::neighbourV(faceIndex, 1);
            MicroBuf::faceCoords(b.faceIndex, p, u, v);
            u = rasterScale*(u + 1.0f);
            v = rasterScale*(v + 1.0f);
            b.ubegin = u - wOn2;  b.uend = u + wOn2;
            b.vbegin = v - wOn2;  b.vend = v + wOn2;
        }
        for(int iface = 0; iface < nfaces; ++iface)
        {
            BoundData& bd = boundData[iface];
            // Range of pixels which the box touches (note, exclusive end)
            int ubeginRas = Imath::clamp(int(bd.ubegin),   0, faceRes);
            int uendRas   = Imath::clamp(int(bd.uend) + 1, 0, faceRes);
            int vbeginRas = Imath::clamp(int(bd.vbegin),   0, faceRes);
            int vendRas   = Imath::clamp(int(bd.vend) + 1, 0, faceRes);
            float* face = microBuf.face(bd.faceIndex);
            for(int iv = vbeginRas; iv < vendRas; ++iv)
            for(int iu = ubeginRas; iu < uendRas; ++iu)
            {
                int pixelIndex = 2*(iv*faceRes + iu);
                assert(pixelIndex < 2*faceRes*faceRes);
                // calculate coverage
                float urange = std::min<float>(iu+1, bd.uend) -
                               std::max<float>(iu,   bd.ubegin);
                float vrange = std::min<float>(iv+1, bd.vend) -
                               std::max<float>(iv,   bd.vbegin);
                // Need to combine these two opacities:
                float o1 = urange*vrange;
                float o2 = face[pixelIndex+1];
                // There's more than one way to combine the coverage.
                //
                // 1) The usual method of compositing.  This assumes
                // that successive layers of geometry are uncorrellated so
                // that each attenuates the layer before, but a bunch of
                // semi-covered layers never result in full opacity.
                //
                // face[pixelIndex+1] = 1 - (1 - o1)*(1 - o2);
                //
                // 2) Add the opacities and clamp.  This is more appropriate
                // if we assume that we have adjacent non-overlapping surfels.
                face[pixelIndex+1] = std::min(1.0f, o1 + o2);
                //
                // 3) Plain old point sampling.  This has more noise than the
                // above, but is also probably more well-founded.
                //
                //if((iu + 0.5f) > bd.ubegin && (iu + 0.5f) < bd.uend &&
                //   (iv + 0.5f) > bd.vbegin && (iv + 0.5f) < bd.vend)
                //    face[pixelIndex+1] = 1;
                float d = face[pixelIndex];
                if(plen < d)
                {
                    face[pixelIndex] = plen;
                }
            }
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
        const float* face = depthBuf.face(f);
        for(int iv = 0; iv < depthBuf.res(); ++iv)
        for(int iu = 0; iu < depthBuf.res(); ++iu, face += 2)
        {
            float u = (0.5f + iu)/depthBuf.res()*2.0f - 1.0f;
            float v = (0.5f + iv)/depthBuf.res()*2.0f - 1.0f;
            float d = dot(MicroBuf::direction(f, u, v), N);
            // FIXME: Add in weight due to texel distance from origin.
            if(d > 0)
            {
                // Accumulate light coming from infinity.
                illum += d*(1.0f - face[1]);
                totWeight += d;
            }
        }
    }
    illum /= totWeight;
    return 1 - illum;
}


/// Visualize sources of light in occlusion calculation (for debugging)
void occlWeight(MicroBuf& depthBuf, const V3f& N)
{
    for(int f = MicroBuf::Face_begin; f < MicroBuf::Face_end; ++f)
    {
        float* face = depthBuf.face(f);
        for(int iv = 0; iv < depthBuf.res(); ++iv)
        for(int iu = 0; iu < depthBuf.res(); ++iu, face += 2)
        {
            float u = (0.5f + iu)/depthBuf.res()*2.0f - 1.0f;
            float v = (0.5f + iv)/depthBuf.res()*2.0f - 1.0f;
            float d = dot(MicroBuf::direction(f, u, v), N);
            if(d > 0)
                face[1] = d*(1.0f - face[1]);
            else
                face[1] = 0;
        }
    }
}


/// Bake occlusion from point array back into point array.
void bakeOcclusion(PointArray& points, int faceRes)
{
    const float eps = 0.1;
    MicroBuf depthBuf(faceRes, 2);
    for(int pIdx = 0, npoints = points.size(); pIdx < npoints; ++pIdx)
    {
        if(pIdx % 100 == 0)
            std::cout << 100.0f*pIdx/points.size() << "%\n";
        float* data = &points.data[pIdx*points.stride];
        // normal of current point
        V3f N = V3f(data[3], data[4], data[5]);
        // position of current point relative to shading point
        V3f P = V3f(data[0], data[1], data[2]);
        float r = data[6];
        depthBuf.reset();
        microRasterize(depthBuf, P + N*r*eps, N, 0, points);
        float occl = occlusion(depthBuf, N);
        data[7] = data[8] = data[9] = 1 - occl;
    }
}


#endif // AQSIS_MICROBUFFER_H_INCLUDED

// vi: set et:
