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

#ifndef MICROBUF_PROJ_FUNC_H_
#define MICROBUF_PROJ_FUNC_H_

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathMath.h>

#include "diffuse/DiffusePointOctree.h"
#include "MicroBuf.h"

namespace Aqsis {

/**
 * Render diffuse surfels into a micro environment buffer.
 *
 * @param integrator
 * 			The integrator for incoming geometry/lighting information.
 * @param P
 * 			Position of light microbuffer.
 * @param N
 * 			Normal of the surface on the postion of the microbuffer (normalized).
 * @param coneAngle
 * 			The cone about the normal N: coneAngle = max angle of interest
 * 			between N and the incoming light.
 * @param maxSolidAngle
 * 			Maximum solid angle allowed for points in interior tree nodes.
 * @param points
 * 			The diffuse point octree containing the surfels to be rendered.
 */
template<typename IntegratorT>
void microRasterize(IntegratorT& integrator, Imath::V3f P, Imath::V3f N,
		float coneAngle, float maxSolidAngle, const DiffusePointOctree& points);


/**
 * Rasterize surfel (disk) into the given integrator
 *
 * @param integrator
 * 			The integrator for incoming geometry/lighting information.
 * @param N
 * 			Normal of the surface on the postion of the microbuffer (normalized).
 * @param p
 * 			Position of the surfel.
 * @param n
 * 			Normal of the surfel.
 * @param r
 * 			Radious of the surfel.
 * @param cosConeAngle
 * 			The cosine of the cone angle, surfels outsides this cone are not rendered.
 * @param sinConeAngle
 * 			The sine of the cone angle, surfels outsides this cone are not rendered.
 */
template<typename IntegratorT>
void renderDisk(IntegratorT& integrator, Imath::V3f N, Imath::V3f p, Imath::V3f n, float r,
                float cosConeAngle, float sinConeAngle);


}
#endif /* MICROBUF_PROJ_FUNC_H_ */
