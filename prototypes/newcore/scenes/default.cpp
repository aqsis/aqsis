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
boost::shared_ptr<Geometry> createPatch(const Vec3& a, const Vec3& b,
                                        const Vec3& c, const Vec3& d,
                                        const Mat4& trans = Mat4())
{
    PrimvarStorageBuilder builder;
    float P[] = {
        a.x, a.y, a.z,  b.x, b.y, b.z,
        c.x, c.y, c.z,  d.x, d.y, d.z,
    };
    builder.add(Primvar::P, P, array_len(P));
    float Cs[] = {
        1, 0, 0,  0, 1, 0,  0, 0, 1,  1, 1, 1
    };
    builder.add(Primvar::Cs, Cs, array_len(Cs));
    IclassStorage storReq(1,4,4,4,4);
    boost::shared_ptr<Geometry> patch(new Patch(builder.build(storReq)));
    patch->transform(trans);
    return patch;
}

static
void addCube(Attributes& attrs, Renderer& r, const Mat4& otow)
{
    r.add(createPatch(Vec3(-1,-1,-1), Vec3(-1,-1,1),
                      Vec3(-1,1,-1), Vec3(-1,1,1), otow), attrs);
    r.add(createPatch(Vec3(1,-1,-1), Vec3(1,-1,1),
                      Vec3(1,1,-1), Vec3(1,1,1), otow), attrs);

    r.add(createPatch(Vec3(-1,-1,-1), Vec3(-1,-1,1),
                      Vec3(1,-1,-1), Vec3(1,-1,1), otow), attrs);
    r.add(createPatch(Vec3(-1,1,-1), Vec3(-1,1,1),
                      Vec3(1,1,-1), Vec3(1,1,1), otow), attrs);

    r.add(createPatch(Vec3(-1,-1,-1), Vec3(-1,1,-1),
                      Vec3(1,-1,-1), Vec3(1,1,-1), otow), attrs);
    r.add(createPatch(Vec3(-1,-1,1), Vec3(-1,1,1),
                      Vec3(1,-1,1), Vec3(1,1,1), otow), attrs);
}

void renderDefaultScene()
{
    Options opts;
    opts.xRes = 1024;
    opts.yRes = 1024;
    opts.gridSize = 8;
    opts.clipNear = 0.1;

    Attributes attrs;
    attrs.shadingRate = 1;
    attrs.smoothShading = true;
    attrs.surfaceShader = createShader("test");

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
