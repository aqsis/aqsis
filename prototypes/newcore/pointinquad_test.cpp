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

#include <algorithm>
#include <cstring>

#include "pointinquad.h"

int main()
{
    // CCW
    Vec3 a = Vec3(5,0,0);
    Vec3 b = Vec3(0,5,0);
    Vec3 c = Vec3(-5,0,0);
    Vec3 d = Vec3(0,-5,0);
//    std::swap(c,d);  // make bow-tie.
//    std::swap(b, d);  // make clockwise
    c *= -0.5;// make arrow
    PointInQuad inQuad(vec2_cast(a), vec2_cast(b),
                       vec2_cast(c), vec2_cast(d));

    const int nx = 32;
    const int ny = 32;

    int counts[nx*ny];
    memset(counts, 0, nx*ny*sizeof(int));

    float minx = -10;
    float miny = -10;
    float maxx = 10;
    float maxy = 10;
    float dx = (maxx - minx)/(nx-1);
    float dy = (maxy - miny)/(ny-1);

    // Rasterize using point-in-polygon tests
    const int nTrials = 1; //40000
    for(int k = 0; k < nTrials; ++k)
    {
        for(int j = 0; j < ny; ++j)
        {
            float y = miny + dy*j;
            for(int i = 0; i < nx; ++i)
            {
                float x = minx + dx*i;
                Sample samp(Vec2(x, y));
                counts[j*nx + i] += inQuad(samp);
            }
        }
    }

    // Print out the rasterized grid.
    for(int j = 0; j < ny; ++j)
    {
        for(int i = 0; i < nx; ++i)
        {
            if(counts[(ny - 1 - j)*nx + i])
                std::cout << "x ";
            else
                std::cout << ". ";
        }
        std::cout << "\n";
    }

    return 0;
}

