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

void renderSimpleDeformationScene()
{
    Options opts;
    opts.xRes = 1024;
    opts.yRes = 1024;
    opts.gridSize = 8;
    opts.clipNear = 0.1;
    opts.superSamp = Imath::V2i(1,1);
    opts.pixelFilter = makeBoxFilter(Vec2(1.0,1.0));
    opts.shutterMax = 0.5;

    Attributes attrs;
    attrs.shadingRate = 1;
    attrs.smoothShading = true;
//    attrs.surfaceShader = createShader("default");

//    Mat4 camToScreen = perspectiveProjection(90, opts.clipNear, opts.clipFar);
    Mat4 camToScreen = screenWindow(-1,2, -1,2);

    // Output variables.
    VarList outVars;
    outVars.push_back(Stdvar::z);

    Renderer renderer(opts, camToScreen, outVars);

    GeometryKeys keys;
    keys.push_back(createPatch(Vec3(0,0,1), Vec3(1,0,2),
                               Vec3(0,1,1), Vec3(1,1,2)));
    keys.push_back(createPatch(Vec3(0.6,0.6,1), Vec3(0.9,0,2),
                               Vec3(0,0.9,0.9), Vec3(0.9,0.9,2)));
    renderer.add(keys, attrs);

    renderer.render();
}

