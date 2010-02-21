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
    IclassStorage storReq(1,4,4,4,4);
    boost::shared_ptr<Geometry> patch(new Patch(builder.build(storReq)));
//    boost::shared_ptr<Geometry> patch(new PatchSimple(a,b,c,d));
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
    opts.filterWidth = Vec2(1,1);

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
