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


/// Determine whether a sphere is wholly outside a cone.
///
/// This is used to cull disks - the sphere is the bounding sphere of the
/// disk, and the cone is the cone of incoming light of interest.
///
/// Getting an expression which is both correct and efficient (ie, not
/// involving special functions) is tricky.  The simple version of it is when
/// the radius is zero, in which case we just need to compute
///
///     cosConeAngle > dot(p/plen, N)
///
/// In the general case of spheres with nonzero radius the condition to check
/// is that
///
///     coneAngle + boundingSphereAngle < coneNormalToSphereCenterAngle
///
/// After some algebra, this reduces to the expression
///
///     sqrt(dot(p,p) - r*r)*cosConeAngle - r*sinConeAngle > dot(p, n);
///
/// which is valid as long as the sum of the sphere angle and cone angle are
/// less than pi:
///
///     sqrt(1 - r*r/dot(p,p)) < -cosConeAngle
///
/// in which case the sphere must intersect the cone.
///
/// \param p - center of sphere
/// \param plen2 - length of p
/// \param r - sphere radius
/// \param n - cone normal
/// \param cosConeAngle - cos(theta) where theta = cone angle from N
/// \param sinConeAngle - sin(theta)
inline bool sphereOutsideCone(V3f p, float plen2, float r,
                              V3f n, float cosConeAngle, float sinConeAngle)
{
    // The actual expressions used here are an optimized version which does
    // the same thing, but avoids calling sqrt().  This makes a difference
    // when this is the primary culling test and you're iterating over lots of
    // points - you need to reject invalid points as fast as possible.
    float x = plen2 - r*r;
    // special case - if the sphere covers the origin, it must intersect the
    // cone.
    if(x < 0)
        return false;
    // Special case - if sphere angle and cone angle add to pi, the sphere and
    // cone must intersect.
    if(cosConeAngle < 0 && x < plen2*cosConeAngle*cosConeAngle)
        return false;
    // General case
    float lhs = x*cosConeAngle*cosConeAngle;
    float rhs = dot(p, n) + r*sinConeAngle;
    return copysignf(lhs, cosConeAngle) > copysignf(rhs*rhs, rhs);
}


/// Utility to solve the quadratic equation
inline void solveQuadratic(float a, float b, float c,
                           float& lowRoot, float& highRoot)
{
    float det = b*b - 4*a*c;
    assert(det > 0);
    float firstTerm = -b/(2*a);
    float secondTerm = sqrtf(det)/(2*a);
    lowRoot = firstTerm - secondTerm;
    highRoot = firstTerm + secondTerm;
}

/// Render a disk into the microbuffer using exact visibility testing
///
/// When two surfaces meet at a sharp angle, the visibility of surfels on the
/// adjacent surface needs to be computed more carefully than with the
/// approximate square-based rasterization method.  If not, disturbing
/// artifacts will appear where the surfaces join.  This function solves the
/// problem by resolving visibility of a disk using ray tracing.
///
/// \param microBuf - buffer to render into.
/// \param p - position of disk center relative to microbuffer
/// \param n - disk normal
/// \param r - disk radius
static void renderDiskExact(MicroBuf& microBuf, V3f p, V3f n, float r)
{
    int faceRes = microBuf.res();
    float plen2 = p.length2();
    if(plen2 == 0) // Sanity check
        return;
    // Angle from face normal to edge is acos(1/sqrt(3)).
    static float cosFaceAngle = 1.0f/sqrt(3);
    static float sinFaceAngle = sqrt(2.0f/3.0f);
    for(int iface = 0; iface < 6; ++iface)
    {
        float* face = microBuf.face(iface);
        // Avoid rendering to the current face if the disk definitely doesn't
        // touch it.  First check the cone angle
        if(sphereOutsideCone(p, plen2, r, MicroBuf::faceNormal(iface),
                             cosFaceAngle, sinFaceAngle))
            continue;
        // Also check whether the disk is on the opposite face.
        if(MicroBuf::dotFaceNormal(iface, p) < 0 &&
           fabs(MicroBuf::dotFaceNormal(iface, n)) > cosFaceAngle)
            continue;
        // Here we compute components of a quadratic function
        //
        //   q(u,v) = a0*u*u + b0*u*v + c0*v*v + d0*u + e0*v + f0
        //
        // such that the disk lies in the region satisfying f(u,v) < 0.  Start
        // with the implicit definition of the disk on the plane,
        //
        //   norm(dot(p,n)/dot(V,n) * V - p)^2 - r^2 < 0
        //
        // and compute coefficients A,B,C such that
        //
        //   A*dot(V,V) + B*dot(V,n)*dot(p,V) + C < 0
        //
        float dot_pn = dot(p,n);
        float A = dot_pn*dot_pn;
        float B = -2*dot_pn;
        float C = plen2 - r*r;
        // Next, project this onto the current face to compute the
        // coefficients a0 through to f0.
        V3f pp = MicroBuf::canonicalFaceCoords(iface, p);
        V3f nn = MicroBuf::canonicalFaceCoords(iface, n);
        float a0 = A + B*nn.x*pp.x + C*nn.x*nn.x;
        float b0 = B*(nn.x*pp.y + nn.y*pp.x) + 2*C*nn.x*nn.y;
        float c0 = A + B*nn.y*pp.y + C*nn.y*nn.y;
        float d0 = (B*(nn.x*pp.z + nn.z*pp.x) + 2*C*nn.x*nn.z);
        float e0 = (B*(nn.y*pp.z + nn.z*pp.y) + 2*C*nn.y*nn.z);
        float f0 = (A + B*nn.z*pp.z + C*nn.z*nn.z);
        // Finally, transform the coefficients so that they define the
        // implicit function in raster face coordinates, (iu, iv)
        float scale = 2.0f/faceRes;
        float scale2 = scale*scale;
        float off = 0.5f*scale - 1.0f;
        float a = scale2*a0;
        float b = scale2*b0;
        float c = scale2*c0;
        float d = ((2*a0 + b0)*off + d0)*scale;
        float e = ((2*c0 + b0)*off + e0)*scale;
        float f = (a0 + b0 + c0)*off*off + (d0 + e0)*off + f0;
        // The coefficients a,b,c,d,e,f may represent an ellipse or a
        // hyperbola in the face's raster coordinates, determine which.
        float det = 4*a*c - b*b;
        if(det > 0)
        {
            // If the coefficients represent an ellipse, we may construct a
            // tight bound.
            int ubegin = 0, uend = faceRes;
            int vbegin = 0, vend = faceRes;
            float ub = 0, ue = 0;
            solveQuadratic(det, 4*d*c - 2*b*e, 4*c*f - e*e, ub, ue);
            ubegin = std::max(0, Imath::ceil(ub));
            uend   = std::min(faceRes, Imath::ceil(ue));
            float vb = 0, ve = 0;
            solveQuadratic(det, 4*a*e - 2*b*d, 4*a*f - d*d, vb, ve);
            vbegin = std::max(0, Imath::ceil(vb));
            vend   = std::min(faceRes, Imath::ceil(ve));
            // By the time we get here, we've expended perhaps 120 FLOPS +
            // 2 sqrts.  The setup is expensive, but the bound is optimal so
            // it will be worthwhile unless the raster faces are very small.
            for(int iv = vbegin; iv < vend; ++iv)
            for(int iu = ubegin; iu < uend; ++iu)
            {
                float q = a*(iu*iu) + b*(iu*iv) + c*(iv*iv) + d*iu + e*iv + f;
                if(q < 0)
                {
                    int pixelIndex = 2*(iv*faceRes + iu);
                    face[pixelIndex+1] = 1;
                }
            }
        }
        else
        {
            // Else, the coefficients do not represent an ellipse (ie, they
            // are hyperbolic or parabolic.)  This is a perfectly valid option
            // which occurs when a disk spans the perspective divide.  In any
            // case, just give up and rasterize the entire face.  It's
            // probably hyperbolic so we must be quite careful to render only
            // the valid branch of the hyperbola.
            for(int iv = 0; iv < faceRes; ++iv)
            for(int iu = 0; iu < faceRes; ++iu)
            {
                if(dot_pn*dot(microBuf.rayDirection(iface, iu, iv), n) < 0)
                    continue;
                float q = a*(iu*iu) + b*(iu*iv) + c*(iv*iv) + d*iu + e*iv + f;
                if(q < 0)
                {
                    // The ray hit the disk, record the hit.
                    int pixelIndex = 2*(iv*faceRes + iu);
                    face[pixelIndex+1] = 1;
                }
            }
        }
        // For reference, all of the tricky rasterization rubbish above could
        // be replaced by the following ray tracing code.  This conceptually
        // simple code would probably be efficient enough if I only knew a
        // good way to compute the tight raster bound!
#if 0
        for(int iv = 0; iv < faceRes; ++iv)
        for(int iu = 0; iu < faceRes; ++iu)
        {
            // d = ray through the pixel.
            V3f d = microBuf.rayDirection(iface, iu, iv);
            // Intersect ray with plane containing disk.
            float t = dot(p, n)/dot(d, n);
            if(t > 0 && (t*d - p).length2() < r*r)
            {
                // The ray hit the disk, record the hit.
                int pixelIndex = 2*(iv*faceRes + iu);
                face[pixelIndex+1] = 1;
            }
        }
#endif
    }
}


void microRasterize(MicroBuf& microBuf, V3f P, V3f N, float coneAngle,
                    const PointArray& points)
{
    int faceRes = microBuf.res();
    float rasterScale = 0.5f*microBuf.res();
    float sinConeAngle = sin(coneAngle);
    float cosConeAngle = cos(coneAngle);
    for(int pIdx = 0, npoints = points.size(); pIdx < npoints; ++pIdx)
    {
        const float* data = &points.data[pIdx*points.stride];
        // normal of current point
        V3f n = V3f(data[3], data[4], data[5]);
        // position of current point relative to shading point
        V3f p = V3f(data[0], data[1], data[2]) - P;
        float plen2 = p.length2();
        float r = data[6];
        // Cull points which lie outside the cone of interest.
        if(sphereOutsideCone(p, plen2, r, N, cosConeAngle, sinConeAngle))
            continue;
        float plen = sqrt(plen2);
        // If distance_to_point / radius is less than exactRenderCutoff,
        // resolve the visibility exactly rather than using a cheap approx.
        //
        // TODO: Adjust exactRenderCutoff for best results!
        const float exactRenderCutoff = 8.0f;
        if(plen < exactRenderCutoff*r)
        {
            // Multiplier for radius to make the cracks a bit smaller.  We
            // can't do this too much, or sharp convex edges will be
            // overoccluded (TODO: Adjust for best results!  Maybe the "too
            // large" problem could be worked around using a tracing offset?)
            const float radiusMultiplier = M_SQRT2;
            // Resolve visibility of very close surfels using ray tracing.
            // This is necessary to avoid artifacts where surfaces meet.
            renderDiskExact(microBuf, p, n, radiusMultiplier*r);
            continue;
        }
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
        microRasterize(depthBuf, P + N*r*eps, N, M_PI_2, points);
        float occl = occlusion(depthBuf, N);
        data[7] = data[8] = data[9] = 1 - occl;
    }
}

// vi: set et:
