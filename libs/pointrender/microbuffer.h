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

#include "pointcloud.h"

#include <cfloat>
#include <cmath>

#include <boost/scoped_array.hpp>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathMath.h>

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

        MicroBuf(int faceRes, int nchans = 1)
            : m_res(faceRes),
            m_nchans(nchans),
            m_faceSize(nchans*faceRes*faceRes),
            m_pixels()
        {
            m_pixels.reset(new float[m_faceSize*Face_end]);
            m_directions.reset(new V3f[Face_end*faceRes*faceRes]);
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

        /// Compute dot product of vec with face normal on given face
        static float dotFaceNormal(int faceIdx, V3f vec)
        {
            assert(faceIdx < Face_end && faceIdx >= Face_begin);
            return (faceIdx < 3) ? vec[faceIdx] : -vec[faceIdx-3];
        }

        /// Get direction vector for pixel on given face.
        V3f rayDirection(int faceIdx, int u, int v) const
        {
            return m_directions[(faceIdx*m_res + v)*m_res + u];
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
        // Pixel face storage
        boost::scoped_array<float> m_pixels;
        // Storage for pixel ray directions
        boost::scoped_array<V3f> m_directions;
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
                    const PointArray& points);


/// Compute ambient occlusion based on depths held in a micro env buffer.
///
/// This is one minus the zero-bounce light coming from infinity to the point.
///
/// \param depthBuf - environment buffer of depths at a point
/// \param N - normal at point
float occlusion(const MicroBuf& depthBuf, const V3f& N);


/// Visualize sources of light in occlusion calculation (for debugging)
void occlWeight(MicroBuf& depthBuf, const V3f& N);


/// Bake occlusion from point array back into point array.
void bakeOcclusion(PointArray& points, int faceRes);



#endif // AQSIS_MICROBUFFER_H_INCLUDED

// vi: set et:
