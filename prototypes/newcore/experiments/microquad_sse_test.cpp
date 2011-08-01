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

#include <cstring>

#include "microquad_sse.h"

int main()
{
    // CCW
    Vec3 a = Vec3(5,0,0);
    Vec3 b = Vec3(0,5,0);
    Vec3 c = Vec3(-5,0,0);
    Vec3 d = Vec3(0,-5,0);
    //PointInQuadSSE inQuadSSE(a,b,c,d);
    PointInQuad inQuad(a,b,c,d);
    PointsInQuad pointsInQuad(a,b,c,d);
    // CW
//    PointInQuad inQuad(Vec3(5,0,0), Vec3(0,-5,0), Vec3(-5,0,0), Vec3(0,5,0));

//    std::cout << quad.bound() << "\n";

    int nx = 32;
    int ny = 32;

//    float counts[nx*ny] __attribute__((aligned(16)));
    int counts[nx*ny] __attribute__((aligned(16)));
    memset(counts, 0, nx*ny*sizeof(int));

    float minx = -10;
    float miny = -10;
    float maxx = 10;
    float maxy = 10;
    float dx = (maxx - minx)/(nx-1);
    float dy = (maxy - miny)/(ny-1);

    float4 px[nx*ny/4];
    float4 py[nx*ny/4];
    int idx = 0;
    for(int j = 0; j < ny; ++j)
    {
        float y = miny + dy*j;
        for(int i = 0; i < nx; i+=4)
        {
            px[idx] = minx + dx*(i + float4(0,1,2,3));
            py[idx] = y;
            ++idx;
        }
    }

    for(int k = 0; k < 100000; ++k)
    {
        int idx = 0;
        for(int j = 0; j < ny; ++j)
        {
            float y = miny + dy*j;
//            for(int i = 0; i < nx; ++i)
//            {
//                float x = minx + dx*i;
//                Sample samp(Vec2(x, y));
//                counts[j*nx + i] += inQuad(samp);
//            }
//            for(int i = 0; i < nx; i+=4)
//            {
//                counts[j*nx + i] += inQuad(Sample(Vec2(minx + dx*i, y)));
//                counts[j*nx + i+1] += inQuad(Sample(Vec2(minx + dx*(i+1), y)));
//                counts[j*nx + i+2] += inQuad(Sample(Vec2(minx + dx*(i+2), y)));
//                counts[j*nx + i+3] += inQuad(Sample(Vec2(minx + dx*(i+3), y)));
//            }
            for(int i = 0; i < nx; i+=4)
            {
//                __m128 px = _mm_set1_ps(minx) + _mm_set1_ps(dx)*
//                            _mm_setr_ps(i, i+1, i+2, i+3);
//                __m128 py = _mm_set1_ps(y);
//                __m128 res = pointsInQuad.contains(px, py);
//                _mm_store_ps((float*)(counts + j*nx+i), res);

//                __m128 res = pointsInQuad.contains(px[idx].get(), py[idx].get());
//                ++idx;
//                _mm_store_ps((float*)(counts + j*nx+i), res);

                int res = pointsInQuad.contains(
                        minx + dx*(i + float4(0,1,2,3)), y);
//                int res = pointsInQuad.contains(px[idx], py[idx]);
//                ++idx;
                counts[j*nx+i]   +=  res & 1;
                counts[j*nx+i+1] +=  (res >> 1) & 1;
                counts[j*nx+i+2] +=  (res >> 2) & 1;
                counts[j*nx+i+3] +=  (res >> 3) & 1;
            }
        }
    }

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

