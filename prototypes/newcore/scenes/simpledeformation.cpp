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

