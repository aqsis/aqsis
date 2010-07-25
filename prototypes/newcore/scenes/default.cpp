// Aqsis
// Copyright (C) 1997 - 2010, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "renderer.h"
#include "surfaces.h"
#include "simple.h"

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

