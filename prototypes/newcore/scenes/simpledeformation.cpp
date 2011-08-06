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

static GeometryPtr createPatch(const float P[12], const float Cs[3],
                               const Mat4& trans = Mat4())
{
    PrimvarStorageBuilder builder;
    builder.add(Primvar::P, P, 12);
    builder.add(PrimvarSpec(PrimvarSpec::Constant, PrimvarSpec::Color, 1, ustring("Cs")),
                Cs, 3);
    IclassStorage storReq(1,4,4,4,4);
    GeometryPtr patch(new Patch(builder.build(storReq)));
    patch->transform(trans);
    return patch;
}

void renderSimpleDeformationScene()
{
    Options opts;
    opts.xRes = 1024;
    opts.yRes = 1024;
    opts.gridSize = 8;
    opts.clipNear = 0.1;
    opts.superSamp = Imath::V2i(4,4);
    opts.pixelFilter = makeSincFilter(Vec2(3.0,3.0));
    opts.shutterMin = 0;
    opts.shutterMax = 2;

    Attributes attrs;
    attrs.shadingRate = 1;
    attrs.smoothShading = true;
    attrs.surfaceShader = createShader("default");

    Mat4 camToScreen = perspectiveProjection(90, opts.clipNear, opts.clipFar);

    // Output variables.
    VarList outVars;
    outVars.push_back(Stdvar::Ci);

    Renderer renderer(opts, camToScreen, outVars);

    {
        float Cs[3] = {1,1,1};
        GeometryKeys keys;
        float P1[12] = {-0.4,-0.4,1,  -0.4,-0.3,1,  -0.3,-0.4,1,  -0.3,-0.3,1};
        keys.push_back(GeometryKey(0, createPatch(P1,Cs)));
        float P2[12] = {-0.1,-0.1,1,  -0.1,0.1,1,  0.1,-0.1,1,  0.1,0.1,1};
        keys.push_back(GeometryKey(2, createPatch(P2,Cs)));
        renderer.add(keys, attrs);
    }

    {
        float Cs[3] = {0.6,0.4,0.4};
        GeometryKeys keys;
        float P1[12] = {1,0,2,  0,1.1,2,  0,-1.1,2,  -1,0,2};
        keys.push_back(GeometryKey(0, createPatch(P1,Cs)));
        float P2[12] = {1.5,0,2,  0,0.8,2,  0,-0.8,2,  -1.5,0,2};
        keys.push_back(GeometryKey(2, createPatch(P2,Cs)));
        renderer.add(keys, attrs);
    }

    renderer.render();
}

