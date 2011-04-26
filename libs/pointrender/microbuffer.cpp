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

#include "microbuffer.h"

#include <OpenEXR/ImathFun.h>


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
static void raytraceDisk(MicroBuf& microBuf, V3f p, V3f n, float r)
{
    int faceRes = microBuf.res();
    float plength = p.length();
    // Raytrace the disk if it's "close" as compared to the disk
    // radius.
    for(int iface = 0; iface < 6; ++iface)
    {
        float faceNormalDotP = MicroBuf::dotFaceNormal(iface, p) / plength;
        // Cull face if we know the bounding cone of the face lies outside the
        // bounding cone of the disk.  TODO: More efficient raster bounding!
        if(faceNormalDotP < -1.0f/sqrt(3.0f))
            continue;
        float* face = microBuf.face(iface);
        for(int iv = 0; iv < faceRes; ++iv)
        for(int iu = 0; iu < faceRes; ++iu)
        {
            // d = ray through the pixel.
            V3f d = microBuf.rayDirection(iface, iu, iv);
            // Intersect ray with plane containing disk.
            float t = dot(p, n)/dot(d, n);
            // Expand the disk just a little, to make the cracks a bit smaller.
            // We can't do this too much, or sharp convex edges will be
            // overoccluded (TODO: Adjust for best results!  Maybe the "too
            // large" problem could be worked around using a tracing offset?)
            const float extraRadiusSqrt = 1.5f;
            if(t > 0 && (t*d - p).length2() < extraRadiusSqrt*r*r)
            {
                // The ray hit the disk, record the hit.
                int pixelIndex = 2*(iv*faceRes + iu);
                face[pixelIndex+1] = 1;
            }
        }
    }
}


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
        // If distance_to_point / radius is less than raytraceCutoff, raytrace
        // the surfel rather than rasterize.
        const float raytraceCutoff = 8.0f;
        if(plen < raytraceCutoff*r)
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
        // 1) The area of the original disk
        float origArea = M_PI*r*r;
        // 2) The angles between the disk normal n, viewing vector p, and face
        // normal.  This is the area projected onto a plane parallel to the env
        // map face, and through the centre of the disk.
        float pDotFaceN = MicroBuf::dotFaceNormal(faceIndex, p);
        float angleFactor = fabs(dot(p, n)/pDotFaceN);
        // 3) Ratio of distance to the surfel vs distance to projected point on
        // the face.
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
            float d = dot(depthBuf.rayDirection(f, iu, iv), N);
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


void occlWeight(MicroBuf& depthBuf, const V3f& N)
{
    for(int f = MicroBuf::Face_begin; f < MicroBuf::Face_end; ++f)
    {
        float* face = depthBuf.face(f);
        for(int iv = 0; iv < depthBuf.res(); ++iv)
        for(int iu = 0; iu < depthBuf.res(); ++iu, face += 2)
        {
            float d = dot(depthBuf.rayDirection(f, iu, iv), N);
            if(d > 0)
                face[1] = d*(1.0f - face[1]);
            else
                face[1] = 0;
        }
    }
}


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

// vi: set et:
