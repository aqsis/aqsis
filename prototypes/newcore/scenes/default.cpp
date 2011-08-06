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

static
void addQuadMesh(Attributes& attrs, Renderer& r, const Mat4& otow,
                 int nfaces, int vertices[],
                 float P_in[], float N_in[] = 0, float Cs_in[] = 0)
{
    DataView<Vec3> P(P_in);
    DataView<Vec3> N(N_in);
    DataView<Col3> Cs(Cs_in);
    float P_stor[12];
    float N_stor[12];
    float Cs_stor[12];
    for(int face = 0; face < nfaces; ++face)
    {
        PrimvarStorageBuilder builder;
#       define COPYVAR(dest, src) \
            dest[0] = src[vertices[0]]; dest[1] = src[vertices[1]]; \
            dest[2] = src[vertices[3]]; dest[3] = src[vertices[2]];
        {
            DataView<Vec3> view(P_stor);
            COPYVAR(view, P);
            builder.add(Primvar::P, P_stor, 12);
        }
        if(N)
        {
            DataView<Vec3> view(N_stor);
            COPYVAR(view, N);
            builder.add(Primvar::N, N_stor, 12);
        }
        if(Cs)
        {
            DataView<Col3> view(Cs_stor);
            COPYVAR(view, Cs);
            builder.add(Primvar::Cs, Cs_stor, 12);
        }
        else
        {
            float Cs_in[] = {1, 1, 1};
            DataView<Col3> Cs(Cs_in, 0);
            DataView<Col3> view(Cs_stor);
            COPYVAR(view, Cs);
            builder.add(Primvar::Cs, Cs_stor, 12);
        }
        IclassStorage storReq(1,4,4,4,4);
        GeometryPtr patch(new Patch(builder.build(storReq)));
        patch->transform(otow);
        r.add(patch, attrs);
        vertices += 4;
    }
}


static
void addCube(Attributes& attrs, Renderer& r, const Mat4& otow)
{
    float P[] = {
        -1, -1, -1,
         1, -1, -1,
        -1,  1, -1,
         1,  1, -1,
        -1, -1,  1,
         1, -1,  1,
        -1,  1,  1,
         1,  1,  1
    };
    float N[] = {
        -1, -1, -1,
         1, -1, -1,
        -1,  1, -1,
         1,  1, -1,
        -1, -1,  1,
         1, -1,  1,
        -1,  1,  1,
         1,  1,  1
    };
    float c = 0.2;
    float Cs[] = {
        c, c, c,
        1, c, c,
        c, 1, c,
        1, 1, c,
        c, c, c,
        1, c, c,
        c, 1, c,
        1, 1, c,
    };
    int vertices[] = {
        0, 2, 3, 1,
        0, 1, 5, 4,
        1, 3, 7, 5,
        3, 2, 6, 7,
        2, 0, 4, 6,
        4, 5, 7, 6,
    };

    addQuadMesh(attrs, r, otow, 6, vertices, P, N, Cs);
}

void renderDefaultScene()
{
    Options opts;
    opts.xRes = 1024;
    opts.yRes = 1024;
    opts.gridSize = 8;
    opts.clipNear = 0.1;
    opts.superSamp = Imath::V2i(4,4);
    opts.pixelFilter = makeSincFilter(Vec2(3.0,3.0));
    opts.fstop = 1;
    opts.focalLength = 0.5;
    opts.focalDistance = 2;

    Attributes attrs;
    attrs.shadingRate = 1;
    attrs.smoothShading = true;
    attrs.surfaceShader = createShader("test");
    attrs.displacementBound = 0.15;

    Mat4 camToScreen = perspectiveProjection(90, opts.clipNear, opts.clipFar);

    // Output variables.
    VarList outVars;
    outVars.push_back(Stdvar::Cs);
    outVars.push_back(Stdvar::z);
    outVars.push_back(Stdvar::Ci);

    Renderer r(opts, camToScreen, outVars);

    // Cube geometry
    addCube(attrs, r,
        Mat4().setAxisAngle(Vec3(0,1,0), deg2rad(45)) *
        Mat4().setAxisAngle(Vec3(0,0,1), deg2rad(10)) *
        Mat4().setAxisAngle(Vec3(1,0,0), deg2rad(45)) *
        Mat4().setTranslation(Vec3(0,0,3))
    );

    r.render();
}

