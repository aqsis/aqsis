// Aqsis
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

#include "renderer.h"
#include "surfaces.h"
#include "util.h"

static GeometryPtr createPatch(const float P[12], const float Cs[3],
                               const Mat4& trans = Mat4())
{
    PrimvarStorageBuilder builder;
    builder.add(Primvar::P, P, 12);
    builder.add(PrimvarSpec(PrimvarSpec::Constant, VarSpec::Color, 1,
                            ustring("Cs")), Cs, 3);
    IclassStorage storReq(1,4,4,4,4);
    GeometryPtr patch(new Patch(builder.build(storReq)));
    patch->transform(trans);
    return patch;
}


void renderDofAmountTest()
{
    Options opts;
    opts.xRes = 320;
    opts.yRes = 240;
    opts.gridSize = 8;
    opts.clipNear = 0.1;
    opts.superSamp = Imath::V2i(10,10);
    opts.pixelFilter = makeGaussianFilter(Vec2(2.0,2.0));
    opts.fstop = 100;
    opts.focalLength = 20;
    opts.focalDistance = 3;

    Attributes attrs;
    attrs.shadingRate = 1;
    attrs.smoothShading = true;

    Mat4 camToScreen =
        perspectiveProjection(90, opts.clipNear, opts.clipFar) *
        screenWindow(-2.33333, 0.33333, -1, 1);

    // Output variables.
    VarList outVars;
    outVars.push_back(Stdvar::Cs);

    Renderer r(opts, camToScreen, outVars);

    Mat4 wToO = Mat4().setTranslation(Vec3(-0.5,-0.5,-1)) *
                Mat4().setAxisAngle(Vec3(0,0,1), deg2rad(45)) *
                Mat4().setTranslation(Vec3(-1.5, 0, 2));

    const float P[12] = {0, 0, 0,  1, 0, 0,  0, 1, 0,  1, 1, 0};
    {
        const float Cs[3] = {1, 0.7, 0.7};
        r.add(createPatch(P, Cs, wToO), attrs);
    }
    {
        const float Cs[3] = {0.7, 1, 0.7};
        r.add(createPatch(P, Cs, Mat4().setTranslation(Vec3(0,0,1))*wToO), attrs);
    }
    {
        const float Cs[3] = {0.7, 0.7, 1};
        r.add(createPatch(P, Cs, Mat4().setTranslation(Vec3(0,0,2))*wToO), attrs);
    }
    {
        const float Cs[3] = {1, 0.7, 0.7};
        r.add(createPatch(P, Cs, Mat4().setTranslation(Vec3(0,0,5))*wToO), attrs);
    }
    {
        const float Cs[3] = {0.7, 1, 0.7};
        r.add(createPatch(P, Cs, Mat4().setTranslation(Vec3(0,0,25))*wToO), attrs);
    }

    r.render();
}
