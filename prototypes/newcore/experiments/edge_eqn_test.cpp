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

#include <iostream>
#include <cstdlib>
#include <iomanip>

// Fastest possible version of an edge equation + numerically independent of
// the order in which the end points are specified.  Unfortunately this doesn't
// have the property that the end points lie exactly on the edge!  (They can
// lie slightly on one side or the other according to floating point errors.)
struct EdgeEqFast
{
    float nx;
    float ny;
    float offset;
    EdgeEqFast(float ax, float ay, float bx, float by)
    {
        nx = -(by-ay);
        ny = bx-ax;
        offset = bx*ay - ax*by;
    }

    bool onedge(float x, float y)
    {
        return nx*x + ny*y == offset;
    }

    bool cinside(float x, float y)
    {
        return nx*x + ny*y >= offset;
    }
    bool coutside(float x, float y)
    {
        return nx*x + ny*y <= offset;
    }
    bool oinside(float x, float y)
    {
        return nx*x + ny*y > offset;
    }
    bool ooutside(float x, float y)
    {
        return nx*x + ny*y < offset;
    }
};

std::ostream& operator<<(std::ostream& out, const EdgeEqFast& e)
{
    out << std::setprecision(10) << e.nx << ","
        << std::setprecision(10) << e.ny << ","
        << std::setprecision(10) << e.offset;
    return out;
}

// The version of the edge equations used in aqsis.  This is slightly slower
// than the above, and can potentially depend on the order in which the
// endpoints are specified.  However, it should have the very desirable
// property that the end points lie exactly on the edge.
struct EdgeEqAq
{
    float nx;
    float ny;
    float px;
    float py;
    EdgeEqAq(float ax, float ay, float bx, float by)
    {
        nx = -(by-ay);
        ny = bx-ax;
        px = bx;
        py = by;
    }

    bool onedge(float x, float y)
    {
        return nx*(x-px) + ny*(y-py) == 0;
    }

    bool cinside(float x, float y)
    {
        return nx*(x-px) + ny*(y-py) >= 0;
    }
    bool coutside(float x, float y)
    {
        return nx*(x-px) + ny*(y-py) <= 0;
    }
    bool oinside(float x, float y)
    {
        return nx*(x-px) + ny*(y-py) > 0;
    }
    bool ooutside(float x, float y)
    {
        return nx*(x-px) + ny*(y-py) < 0;
    }
};

typedef EdgeEqAq EdgeEq;
//typedef EdgeEqFast EdgeEq;

float rnd()
{
    return 2*float(std::rand())/RAND_MAX - 1;
}

int main(int argc, char* argv[])
{
//    int seed = 0;
//    std::cin >> seed;
//    srand(seed);

    const int boxsize = 30;
    const float scale = 1e-5;
//    int seed = 2103; // 1996 2004 2044 2057 2060 2064
    for(int seed = 0; seed < 1000; ++seed)
    {
        // Test well-behavedness of edge equations for micropolygon sampling.
        // We construct the following geometry:
        //
        //      b
        //      |
        //  d---e---a
        //      |
        //      c
        //
        // the vertices a,b,c,d surround the vertex e and (contrary to the
        // diagram) are slightly randomly jittered according to the current
        // value of seed.
        //
        // An important property for sampling is that all the edges should
        // actually meet at e - if they don't, we can get multiple sample hits
        // or holes in the centre.
        srand(seed);
        const float amp = 0.2;
        const float ax = 1, ay = amp*rnd();
        const float bx = amp*rnd(), by = 1;
        const float cx = -1, cy = amp*rnd();
        const float dx = amp*rnd(), dy = -1;
        const float ex = 1000+ 0.001*rnd(), ey = -1000+ 0.001*rnd();
        EdgeEq e1(ex,ey, ax+ex,ay+ey);
        EdgeEq e2(ex,ey, bx+ex,by+ey);
        EdgeEq e3(ex,ey, cx+ex,cy+ey);
        EdgeEq e4(ex,ey, dx+ex,dy+ey);
        for(int k = 1; k < 2; ++k)
        {
            const bool print = false;
            int badhits = 1;
            for(int j = boxsize; j >= 0; --j)
            {
                for(int i = 0; i <= boxsize; ++i)
                {
                    float x = (i-boxsize/2)*scale + ex;
                    float y = (j-boxsize/2)*scale + ey;

                    bool hit1 = e1.oinside(x, y) && e2.coutside(x,y);
                    bool hit2 = e2.oinside(x, y) && e3.ooutside(x,y);
                    bool hit3 = e3.cinside(x, y) && e4.ooutside(x,y);
                    bool hit4 = e4.cinside(x, y) && e1.coutside(x,y);
                    int hits = hit1 + hit2 + hit3 + hit4;
                    if(print)
                    {
                        std::cout << hits << " ";
                    }
                    else
                    {
                        if(hits != 1)
                            badhits = hits;
                    }
                }
                if(print)
                    std::cout << "\n";
            }
            if(print)
                std::cout << "--------------------------------------------------\n";
            else if(badhits != 1)
                std::cout << seed << "\n";
        }
        if(!e1.onedge(ex, ey) || !e1.onedge(ax+ex, ay+ey)) std::cout << "e1 end points not on edge " << seed << "\n";
        if(!e2.onedge(ex, ey) || !e2.onedge(bx+ex, by+ey)) std::cout << "e2 end points not on edge " << seed << "\n";
        if(!e3.onedge(ex, ey) || !e3.onedge(cx+ex, cy+ey)) std::cout << "e3 end points not on edge " << seed << "\n";
        if(!e4.onedge(ex, ey) || !e4.onedge(dx+ex, dy+ey)) std::cout << "e4 end points not on edge " << seed << "\n";
    }

//    std::cout << e1 << "   " << e2 << "   "
//              << e3 << "   " << e4 << "\n";

    return 0;
}
