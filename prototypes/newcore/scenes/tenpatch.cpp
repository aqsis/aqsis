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

static GeometryPtr createPatch(const Vec3& a, const Vec3& b,
                               const Vec3& c, const Vec3& d,
                               const Mat4& trans = Mat4())
{
    PrimvarStorageBuilder builder;
    float P[] = {
        a.x, a.y, a.z,  b.x, b.y, b.z,
        c.x, c.y, c.z,  d.x, d.y, d.z,
    };
    builder.add(Primvar::P, P, array_len(P));
    IclassStorage storReq(1,4,4,4,4);
    GeometryPtr patch(new Patch(builder.build(storReq)));
    patch->transform(trans);
    return patch;
}


void renderTenPatchScene()
{
    Options opts;
    opts.xRes = 1024;
    opts.yRes = 1024;
    opts.gridSize = 8;
    opts.clipNear = 0.1;
    opts.superSamp = Imath::V2i(1,1);
    opts.pixelFilter = makeBoxFilter(Vec2(1,1));

    Attributes attrs;
    attrs.shadingRate = 1;
    attrs.smoothShading = true;

    Mat4 camToScreen = screenWindow(0,0.5, 0,0.5);

    // Output variables.
    VarList outVars;
    outVars.push_back(Stdvar::z);

    Renderer r(opts, camToScreen, outVars);

    for(int i = 0; i < 10; ++i)
    {
        r.add(createPatch(Vec3(0.2,0.2,5), Vec3(0.5,-0.5,1),
                          Vec3(-0.5,0.5,1), Vec3(0.5,0.5,5)), attrs);
    }

    r.render();
}
