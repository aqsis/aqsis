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


#ifndef AQSIS_POINTCLOUD_H_INCLUDED
#define AQSIS_POINTCLOUD_H_INCLUDED

#include <cassert>

#include <OpenEXR/ImathVec.h>
#include <boost/shared_ptr.hpp>

using Imath::V3f;

/// Array of surface elements
///
/// TODO: Make into a class, etc.
///
/// Point data is stored in a flat array as
///
///     [P1 N1 r1 user1  P2 N2 r2 user2  ... ]
///
/// where user1 user2... is extra "user data" appended after the position
/// normal, and radius data.
///
struct PointArray
{
    int stride;
    std::vector<float> data;

    /// Get number of points in cloud
    size_t size() const { return data.size()/stride; }

    /// Get centroid of point cloud.
    V3f centroid() const
    {
        V3f sum(0);
        for(std::vector<float>::const_iterator p = data.begin();
            p < data.end(); p += stride)
        {
            sum += V3f(p[0], p[1], p[2]);
        }
        return (1.0f/data.size()*stride) * sum;
    }
};


//------------------------------------------------------------------------------
// Musings on abstraction for point array class... not sure what the best
// abstraction is yet.

#if 0

/// Reference to data for a single surface element
class Point
{
    public:
        Point(const float* data) : m_data(data) {}
        /// Get point position
        V3f P() const { return V3f(m_data[0], m_data[1], m_data[2]); }
        /// Get normal
        V3f N() const { return V3f(m_data[3], m_data[4], m_data[5]); }
        /// Get radius
        float r() const { return m_data[6]; }
        /// Get any additional attached data
        const float* userData() const { return &m_data[7]; }
    private:
        const float* m_data;
};


/// Array of surface elements ("points")
class PointArray
{
    public:
        PointArray(int stride)
            : m_stride(stride)
        { }

        const Point operator[](size_t i) const
        {
            assert(i < data.size());
            return &data[i*m_stride];
        }

        int stride() const { return m_stride; }

        void addPoint(const float* data)
        {
            m_data.insert(m_data.end(), data, data + m_stride);
        }

        // Get centroid of the point cloud.
        V3f centroid() const
        {
            V3f sum(0);
            for(std::vector<float>::const_iterator p = data.begin();
                p < data.end(); p += stride)
            {
                sum += V3f(p[0], p[1], p[2]);
            }
            return (1.0f/data.size()*stride) * sum;
        }

    private:
//        struct Channel
//        {
//            int index;
//            int size;
//            std::string name;
//        };

        int m_stride;
//        std::vector<Channel> m_channelData;
        std::vector<float> m_data;
};
#endif


#endif // AQSIS_POINTCLOUD_H_INCLUDED

// vi: set et:
