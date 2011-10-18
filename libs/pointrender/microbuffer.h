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


#ifndef AQSIS_MICROBUFFER_H_INCLUDED
#define AQSIS_MICROBUFFER_H_INCLUDED

#include "pointcontainer.h"

#include <cfloat>
#include <cmath>
#include <cstring>

#include <boost/scoped_array.hpp>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathMath.h>


namespace Aqsis {

using Imath::V3f;

inline float dot(V3f a, V3f b)
{
    return a^b;
}

/// An axis-aligned cube environment buffer.
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
/// The orientation of the faces is chosen so that they have a consistent
/// coordinate system when laid out in the following unfolded net of faces,
/// viewed from the inside of the cube.  (NB that for other possible nets the
/// coordinates of neighbouring faces won't be consistent.)
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

        /// faceRes gives face resolution.  Faces are square.  nchans gives
        /// the number of channels in each pixel, and defaultPix gives the
        /// values of the channels which will be used when reset() is called.
        MicroBuf(int faceRes, int nchans, const float* defaultPix)
            : m_res(faceRes),
            m_nchans(nchans),
            m_faceSize(nchans*faceRes*faceRes),
            m_pixels()
        {
            m_pixels.reset(new float[m_faceSize*Face_end]);
            m_defaultPixels.reset(new float[m_faceSize*Face_end]);
            m_directions.reset(new V3f[Face_end*faceRes*faceRes]);
            m_pixelSizes.reset(new float[m_faceSize]);
            // Cache direction vectors
            for(int face = 0; face < Face_end; ++face)
            {
                for(int iv = 0; iv < m_res; ++iv)
                for(int iu = 0; iu < m_res; ++iu)
                {
                    // directions of pixels go through pixel centers
                    float u = (0.5f + iu)/faceRes*2.0f - 1.0f;
                    float v = (0.5f + iv)/faceRes*2.0f - 1.0f;
                    m_directions[(face*m_res + iv)*m_res + iu] =
                        direction(face, u, v);
                }
            }
            for(int iv = 0; iv < m_res; ++iv)
            for(int iu = 0; iu < m_res; ++iu)
            {
                float u = (0.5f + iu)/faceRes*2.0f - 1.0f;
                float v = (0.5f + iv)/faceRes*2.0f - 1.0f;
                m_pixelSizes[iv*m_res + iu] = 1.0f/V3f(u,v,1).length2();
            }
            float* pix = m_defaultPixels.get();
            for(int i = 0, iend = size(); i < iend; ++i, pix += m_nchans)
                for(int c = 0; c < m_nchans; ++c)
                    pix[c] = defaultPix[c];
        }

        /// Reset buffer to default (non-rendered) state.
        void reset()
        {
            memcpy(m_pixels.get(), m_defaultPixels.get(),
                   sizeof(float)*size()*m_nchans);
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
            p = canonicalFaceCoords(faceIdx, p);
            assert(p.z != 0);
            float zinv = 1.0/p.z;
            u = p.x*zinv;
            v = p.y*zinv;
        }

        /// Compute dot product of vec with face normal on given face
        static float dotFaceNormal(int faceIdx, V3f vec)
        {
            assert(faceIdx < Face_end && faceIdx >= Face_begin);
            return (faceIdx < 3) ? vec[faceIdx] : -vec[faceIdx-3];
        }

        /// Compute face normal
        static V3f faceNormal(int faceIdx)
        {
            static V3f normals[6] = {
                V3f(1,0,0), V3f(0,1,0), V3f(0,0,1),
                V3f(-1,0,0), V3f(0,-1,0), V3f(0,0,-1)
            };
            return normals[faceIdx];
        }

        /// Get direction vector for pixel on given face.
        V3f rayDirection(int faceIdx, int u, int v) const
        {
            return m_directions[(faceIdx*m_res + v)*m_res + u];
        }

        /// Return relative size of pixel.
        ///
        /// Compared to a pixel in the middle of the cube face, pixels in the
        /// corners of the cube have a smaller angular size.  We must take
        /// this into account when integrating the radiosity.
        ///
        /// \param u,v - face coordinates
        float pixelSize(int u, int v) const
        {
            return m_pixelSizes[m_res*v + u];
        }

        /// Reorder vector components into "canonical face coordinates".
        ///
        /// The canonical coordinates correspond to the coordinates on the +z
        /// face.  If we let the returned vector be q then (q.x, q.y)
        /// correspond to the face (u, v) coordinates, and q.z corresponds to
        /// the signed depth out from the face.
        static V3f canonicalFaceCoords(int faceIdx, V3f p)
        {
            switch(faceIdx)
            {
                case Face_xp: return V3f(-p.z,  p.y, p.x);
                case Face_xn: return V3f(-p.z, -p.y, p.x);
                case Face_yp: return V3f( p.x, -p.z, p.y);
                case Face_yn: return V3f(-p.x, -p.z, p.y);
                case Face_zp: return V3f( p.x,  p.y, p.z);
                case Face_zn: return V3f( p.x, -p.y, p.z);
                default: assert(0 && "invalid face"); return V3f();
            }
        }

        /// Face side resolution
        int res() const { return m_res; }
        /// Number of channels per pixel
        int nchans() const { return m_nchans; }
        /// Total size of all faces in number of texels
        int size() const { return Face_end*m_res*m_res; }

    private:
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

        /// Square face resolution
        int m_res;
        /// Number of channels per pixel
        int m_nchans;
        /// Number of floats needed to store a face
        int m_faceSize;
        /// Pixel face storage
        boost::scoped_array<float> m_pixels;
        boost::scoped_array<float> m_defaultPixels;
        /// Storage for pixel ray directions
        boost::scoped_array<V3f> m_directions;
        /// Pixels on a unit cube are not all equal in angular size
        boost::scoped_array<float> m_pixelSizes;
};


//------------------------------------------------------------------------------
/// Integrator for ambient occlusion.
///
/// The job of an integrator class is to save the microrasterized data
/// somewhere (presumably in a microbuffer) and integrate it at the end of the
/// rasterization to give the final illumination value.
class OcclusionIntegrator
{
    public:
        /// Create integrator with given resolution of the environment map
        /// faces.
        OcclusionIntegrator(int faceRes)
            : m_buf(faceRes, 1, defaultPixel()),
            m_face(0)
        {
            clear();
        }

        /// Get direction of the ray
        V3f rayDirection(int iface, int u, int v)
        {
            return m_buf.rayDirection(iface, u, v);
        }

        /// Get at the underlying buffer
        const MicroBuf& microBuf()
        {
            return m_buf;
        }

        /// Get desired resolution of environment map faces
        int res()
        {
            return m_buf.res();
        }

        /// Reset buffer to default state
        void clear()
        {
            m_buf.reset();
        };

        /// Set extra data (eg, radiosity) associated with the current point
        /// being shaded.
        void setPointData(const float*) { }

        /// Set the face to which subsequent calls of addSample will apply
        void setFace(int iface)
        {
            m_face = m_buf.face(iface);
        };

        /// Add a rasterized sample to the current face
        ///
        /// \param u,v - coordinates of face
        /// \param distance - distance to sample
        /// \param coverage - estimate of pixel coverage due to this sample
        void addSample(int u, int v, float distance, float coverage)
        {
            float* pix = m_face + (v*m_buf.res() + u) * m_buf.nchans();
            // There's more than one way to combine the coverage.
            //
            // 1) The usual method of compositing.  This assumes that
            // successive layers of geometry are uncorrellated so that each
            // attenuates the layer before, but a bunch of semi-covered layers
            // never result in full opacity.
            //
            // 1 - (1 - o1)*(1 - o2);
            //
            // 2) Add the opacities (and clamp to 1 at the end).  This is more
            // appropriate if we assume that we have adjacent non-overlapping
            // surfels.
            pix[0] += coverage;
        }

        /// Compute ambient occlusion based on previously sampled scene.
        ///
        /// This is one minus the zero-bounce light coming from infinity to the
        /// point.
        ///
        /// N is the shading normal; coneAngle is the angle over which to
        /// consider the occlusion in radians.  If coneAngle is not PI/2
        /// radians, the usual cos(theta) weighting is adjusted to
        /// (cos(theta) - cos(coneAngle)) so that it falls continuously to
        /// zero when theta == coneAngle
        float occlusion(V3f N, float coneAngle) const
        {
            // Integrate over face to get occlusion.
            float occ = 0;
            float totWeight = 0;
            float cosConeAngle = std::cos(coneAngle);
            for(int f = MicroBuf::Face_begin; f < MicroBuf::Face_end; ++f)
            {
                const float* face = m_buf.face(f);
                for(int iv = 0; iv < m_buf.res(); ++iv)
                for(int iu = 0; iu < m_buf.res(); ++iu, face += m_buf.nchans())
                {
                    float d = dot(m_buf.rayDirection(f, iu, iv), N) -
                              cosConeAngle;
                    if(d > 0)
                    {
                        d *= m_buf.pixelSize(iu, iv);
                        // Accumulate light coming from infinity.
                        occ += d*std::min(1.0f, face[0]);
                        totWeight += d;
                    }
                }
            }
            if(totWeight != 0)
                occ /= totWeight;
            return occ;
        }

    private:
        static float* defaultPixel()
        {
            static float def[1] = {0};
            return def;
        }

        MicroBuf m_buf;
        float* m_face;
};


//------------------------------------------------------------------------------
/// Microbuffer integrator for radiosity
class RadiosityIntegrator
{
    public:
        /// Create integrator with given resolution of the environment map
        /// faces.
        RadiosityIntegrator(int faceRes)
            : m_buf(faceRes, 5, defaultPixel()),
            m_face(0),
            m_currRadiosity(0)
        {
            clear();
        }

        /// Get direction of the ray
        V3f rayDirection(int iface, int u, int v)
        {
            return m_buf.rayDirection(iface, u, v);
        }

        /// Get at the underlying buffer
        const MicroBuf& microBuf()
        {
            return m_buf;
        }

        /// Get desired resolution of environment map faces
        int res()
        {
            return m_buf.res();
        }

        /// Reset buffer to default state
        void clear()
        {
            m_buf.reset();
        };

        /// Set extra data (eg, radiosity) associated with the current point
        /// being shaded.
        void setPointData(const float* radiosity)
        {
            m_currRadiosity = C3f(radiosity[0], radiosity[1], radiosity[2]);
        }

        /// Set the face to which subsequent calls of addSample will apply
        void setFace(int iface)
        {
            m_face = m_buf.face(iface);
        };

        /// Add a rasterized sample to the current face
        ///
        /// \param u,v - coordinates of face
        /// \param distance - distance to sample
        /// \param coverage - estimate of pixel coverage due to this sample
        void addSample(int u, int v, float distance, float coverage)
        {
            float* pix = m_face + (v*m_buf.res() + u) * m_buf.nchans();
            // TODO: Eventually remove dist if not needed
            float& currDist = pix[0];
            float& currCover = pix[1];
            C3f& radiosity = *reinterpret_cast<C3f*>(pix + 2);
            if(distance < currDist)
                currDist = distance;
            if(currCover < 1)
            {
                if(currCover + coverage <= 1)
                {
                    radiosity += coverage*m_currRadiosity;
                    currCover += coverage;
                }
                else
                {
                    radiosity += (1 - currCover)*m_currRadiosity;
                    currCover = 1;
                }
            }
        }

        /// Integrate radiosity based on previously sampled scene.
        ///
        /// N is the shading normal; coneAngle is the angle over which to
        /// consider the occlusion in radians.  If coneAngle is not PI/2
        /// radians, the usual cos(theta) weighting is adjusted to
        /// (cos(theta) - cos(coneAngle)) so that it falls continuously to
        /// zero when theta == coneAngle
        ///
        /// If occlusion is non-null, the amount of occlusion will also be
        /// computed and stored.
        C3f radiosity(V3f N, float coneAngle, float* occlusion = 0) const
        {
            // Integrate incoming light with cosine weighting to get outgoing
            // radiosity
            C3f rad(0);
            float totWeight = 0;
            float cosConeAngle = std::cos(coneAngle);
            float occ = 0;
            for(int f = MicroBuf::Face_begin; f < MicroBuf::Face_end; ++f)
            {
                const float* face = m_buf.face(f);
                for(int iv = 0; iv < m_buf.res(); ++iv)
                for(int iu = 0; iu < m_buf.res(); ++iu, face += m_buf.nchans())
                {
                    float d = dot(m_buf.rayDirection(f, iu, iv), N) -
                              cosConeAngle;
                    if(d > 0)
                    {
                        d *= m_buf.pixelSize(iu, iv);
                        C3f& radiosity = *(C3f*)(face + 2);
                        rad += d*radiosity;
                        occ += d*face[1];
                        totWeight += d;
                    }
                }
            }
            if(totWeight != 0)
            {
                occ /= totWeight;
                rad = (1.0f/totWeight) * rad;
            }
            if(occlusion)
                *occlusion = occ;
            return rad;
        }

    private:
        static float* defaultPixel()
        {
            // depth, foreground_coverage, foreground_rgb, background_rgb
            static float def[] = {FLT_MAX, 0, 0, 0, 0, 0, 0, 0};
            return def;
        }

        MicroBuf m_buf;
        float* m_face;
        C3f m_currRadiosity;
};


//------------------------------------------------------------------------------
/// Render points into a micro environment buffer.
///
/// \param integrator - integrator for incoming geometry/lighting information
/// \param P - position of light probe
/// \param N - normal for light probe (should be normalized)
/// \param coneAngle - defines cone about N: coneAngle = max angle of interest
///                    between N and the incoming light.
/// \param maxSolidAngle - Maximum solid angle allowed for points in interior
///                    tree nodes.
/// \param points - point cloud to render
template<typename IntegratorT>
void microRasterize(IntegratorT& integrator, V3f P, V3f N, float coneAngle,
                    float maxSolidAngle, const PointOctree& points);


} // namespace Aqsis

#endif // AQSIS_MICROBUFFER_H_INCLUDED

// vi: set et:
