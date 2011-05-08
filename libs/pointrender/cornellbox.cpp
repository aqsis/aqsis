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


#include <vector>
#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathMatrix.h>

#include "cornellbox.h"
#include "pointcontainer.h"

namespace Aqsis {

using Imath::V3f;


inline V3f ptov3(const float* d)
{
    return V3f(d[0], d[1], d[2]);
}


template<typename T>
inline T bilerp(float u, float v, T x1, T x2, T x3, T x4)
{
    return (1-v)*((1-u)*x1 + u*x2) + v*((1-u)*x3 + u*x4);
}


/// Generate a set of points from a bilinear patch
static void makeBilinearPatch(std::vector<float>& outData,
                              const float* patchData, int dataLen,
                              float spatialRes, float scale)
{
    // Get positions of points
    const float* d1 = patchData;
    const float* d2 = patchData + dataLen;
    const float* d3 = patchData + 2*dataLen;
    const float* d4 = patchData + 3*dataLen;
    V3f p1 = scale*ptov3(d1); V3f p2 = scale*ptov3(d2);
    V3f p3 = scale*ptov3(d3); V3f p4 = scale*ptov3(d4);
//    Imath::M44f xform; xform.setAxisAngle(V3f(0,1,0), 0.2f);
//    p1*=xform; p2*=xform; p3*=xform; p4*=xform;
    spatialRes *= scale;
    int nu = int(std::max((p1 - p2).length(), (p3 - p4).length()) / spatialRes);
    int nv = int(std::max((p1 - p3).length(), (p2 - p4).length()) / spatialRes);
    float du = 1.0f/nu;
    float dv = 1.0f/nv;
    for(int j = 0; j < nv; ++j)
    for(int i = 0; i < nu; ++i)
    {
        float u = (0.5f + i)*du;
        float v = (0.5f + j)*dv;
        // Compute position
        V3f p = bilerp(u,v, p1, p2, p3, p4);
        // Compute normal - dp/du x dp/dv
        V3f dpdu = (1-v)*(p2-p1) + v*(p4-p3);
        V3f dpdv = (1-u)*(p3-p1) + u*(p4-p2);
        V3f n = (dpdu % dpdv).normalized();
        // Radius: half length of microquad diagonal
        //float r = 0.5f*(dpdu*du + dpdv*dv).length();
        // Radius of a disk with area of the parallelogram
        float r = sqrt(du*dv*(dpdu % dpdv).length()/M_PI);
        // Save data
        outData.push_back(p.x); outData.push_back(p.y); outData.push_back(p.z);
        outData.push_back(n.x); outData.push_back(n.y); outData.push_back(n.z);
        outData.push_back(r);
        // Finally, fill in any extra "user data" (colour etc)
        for(int k = 3; k < dataLen; ++k)
            outData.push_back(bilerp(u,v, d1[k], d2[k], d3[k], d4[k]));
    }
}


boost::shared_ptr<PointArray> cornellBoxPoints(float spatialRes)
{
    boost::shared_ptr<PointArray> points(new PointArray());
    const int userDataLen = 3;
    const float scale = 0.01;
    points->stride = 7 + userDataLen;
    const int dataLen = userDataLen + 3;
#define PATCH_BEGIN                                          \
    {                                                        \
        float d[] = {
#define PATCH_END                                            \
        };                                                   \
        makeBilinearPatch(points->data, d, dataLen, spatialRes, scale); \
    }

    //--------------------------------------------------
    // Container
    // floor
    PATCH_BEGIN
        552.8, 0, 0,          1,1,1,
        0,     0, 0,          1,1,1,
        549.6, 0, 559.2,      1,1,1,
        0,     0, 559.2,      1,1,1
    PATCH_END
    // ceiling
    PATCH_BEGIN
        556.0, 548.8, 0,      1,1,1,
        556.0, 548.8, 559.2,  1,1,1,
        0,     548.8, 0.0,    1,1,1,
        0,     548.8, 559.2,  1,1,1
    PATCH_END
    // Back wall
    PATCH_BEGIN
        549.6, 0, 559.2,      1,1,1,
        0,     0, 559.2,      1,1,1,
        556.0, 548.8, 559.2,  1,1,1,
        0,     548.8, 559.2,  1,1,1
    PATCH_END
    // Left wall
    // Color [0.64 0.15 0.1]
    PATCH_BEGIN
        552.8, 0,     0,      0.64, 0.15, 0.1,
        549.6, 0,     559.2,  0.64, 0.15, 0.1,
        556,   548.8, 0,      0.64, 0.15, 0.1,
        556,   548.8, 559.2,  0.64, 0.15, 0.1,
    PATCH_END
    // Right wall
    // Color [0.15 0.5 0.15]
    PATCH_BEGIN
        0, 0,     559.2,      0.15, 0.5, 0.15,
        0, 0,     0,          0.15, 0.5, 0.15,
        0, 548.8, 559.2,      0.15, 0.5, 0.15,
        0, 548.8, 0,          0.15, 0.5, 0.15
    PATCH_END

    //--------------------------------------------------
    // Tall block
    PATCH_BEGIN
        423.0, 330.0, 247.0,    1,1,1,
        265.0, 330.0, 296.0,    1,1,1,
        472.0, 330.0, 406.0,    1,1,1,
        314.0, 330.0, 456.0,    1,1,1
    PATCH_END
    PATCH_BEGIN
        423.0,   0.0, 247.0,    1,1,1,
        423.0, 330.0, 247.0,    1,1,1,
        472.0,   0.0, 406.0,    1,1,1,
        472.0, 330.0, 406.0,    1,1,1
    PATCH_END
    PATCH_BEGIN
        472.0,   0.0, 406.0,    1,1,1,
        472.0, 330.0, 406.0,    1,1,1,
        314.0,   0.0, 456.0,    1,1,1,
        314.0, 330.0, 456.0,    1,1,1
    PATCH_END
    PATCH_BEGIN
        314.0,   0.0, 456.0,    1,1,1,
        314.0, 330.0, 456.0,    1,1,1,
        265.0,   0.0, 296.0,    1,1,1,
        265.0, 330.0, 296.0,    1,1,1
    PATCH_END
    PATCH_BEGIN
        265.0,   0.0, 296.0,    1,1,1,
        265.0, 330.0, 296.0,    1,1,1,
        423.0,   0.0, 247.0,    1,1,1,
        423.0, 330.0, 247.0,    1,1,1
    PATCH_END

    //--------------------------------------------------
    // Short block
    PATCH_BEGIN
        130.0, 165.0,  65.0,    1,1,1,
         82.0, 165.0, 225.0,    1,1,1,
        290.0, 165.0, 114.0,    1,1,1,
        240.0, 165.0, 272.0,    1,1,1
    PATCH_END
    PATCH_BEGIN
        290.0,   0.0, 114.0,    1,1,1,
        290.0, 165.0, 114.0,    1,1,1,
        240.0,   0.0, 272.0,    1,1,1,
        240.0, 165.0, 272.0,    1,1,1
    PATCH_END
    PATCH_BEGIN
        130.0,   0.0,  65.0,    1,1,1,
        130.0, 165.0,  65.0,    1,1,1,
        290.0,   0.0, 114.0,    1,1,1,
        290.0, 165.0, 114.0,    1,1,1
    PATCH_END
    PATCH_BEGIN
         82.0,   0.0, 225.0,    1,1,1,
         82.0, 165.0, 225.0,    1,1,1,
        130.0,   0.0,  65.0,    1,1,1,
        130.0, 165.0,  65.0,    1,1,1
    PATCH_END
    PATCH_BEGIN
        240.0,   0.0, 272.0,    1,1,1,
        240.0, 165.0, 272.0,    1,1,1,
         82.0,   0.0, 225.0,    1,1,1,
         82.0, 165.0, 225.0,    1,1,1
    PATCH_END

    return points;
}

} // namespace Aqsis

// vi: set et:
