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

#include <algorithm>
#include <cstring>

#include "pointinquad.h"

using namespace Aqsis;

int main()
{
    // CCW
    V3f a = V3f(5,0,0);
    V3f b = V3f(0,5,0);
    V3f c = V3f(-5,0,0);
    V3f d = V3f(0,-5,0);
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
                Sample samp(V2f(x, y));
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

