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

void renderMbNoiseTestScene()
{
    Options opts;
    opts.xRes = 512;
    opts.yRes = 512;
    opts.gridSize = 8;
    opts.clipNear = 0.1;
    opts.superSamp = Imath::V2i(1,1);
    opts.pixelFilter = makeBoxFilter(Vec2(1,1));
    opts.shutterMin = 0;
    opts.shutterMax = 1;

    Attributes attrs;
    attrs.shadingRate = 1;
    attrs.smoothShading = true;
    attrs.surfaceShader = createShader("default");

    Mat4 camToScreen = screenWindow(0,1, 0,1);

    // Output variables.
    VarList outVars;
    outVars.push_back(Stdvar::Ci);

    Renderer renderer(opts, camToScreen, outVars);

    float Cs[3] = {1,1,1};
    GeometryKeys keys;
    float P1[12] = {0,0,1,  0,1,1,  1,0,1,  1,1,1};
    keys.push_back(GeometryKey(0, createPatch(P1,Cs)));
    float P2[12] = {1,0,1,  1,1,1,  2,0,1,  2,1,1};
    keys.push_back(GeometryKey(1, createPatch(P2,Cs)));
    renderer.add(keys, attrs);

    renderer.render();
}

