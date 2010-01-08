#include <iostream>
#include <cstdio>
#include <cstdlib>

#include <OpenEXR/ImathVec.h>

typedef Imath::V2f Vec2;

float cross(Vec2 a, Vec2 b)
{
    return a.x*b.y - b.x*a.y;
}


struct EdgeEqn
{
    float mx;
    float my;
    float off;

    EdgeEqn(Vec2 a, Vec2 b)
    {
        Vec2 e = b-a;
        mx = -e.y;
        my = e.x;
        off = cross(b,a);
    }

    bool inside(Vec2 p) const
    {
        return mx*p.x + my*p.y >= off;
    }
    bool insideNoBdry(Vec2 p) const
    {
        return mx*p.x + my*p.y > off;
    }
};


struct EdgeEqn2
{
    float mx;
    float my;
    float offx;
    float offy;

    EdgeEqn2(Vec2 a, Vec2 b)
    {
        Vec2 e = b-a;
        mx = -e.y;
        my = e.x;
        offx = a.x;
        offy = a.y;
    }

    bool inside(Vec2 p) const
    {
        return mx*(p.x-offx) + my*(p.y-offy) >= 0;
    }
    bool insideNoBdry(Vec2 p) const
    {
        return mx*(p.x-offx) + my*(p.y-offy) > 0;
    }
};


template<typename EdgeT>
Vec2 findBdry(const EdgeT& e, float y, float x1, float x2)
{
    bool ins1 = e.inside(Vec2(x1, y));
    bool ins2 = e.inside(Vec2(x2, y));
    while(true)
    {
        float xNew = (x1+x2)/2;
        bool ins = e.inside(Vec2(xNew, y));
        if(ins == ins1)
        {
            if(x1 == xNew)
                break;
            x1 = xNew;
        }
        else
        {
            if(x2 == xNew)
                break;
            x2 = xNew;
        }
    }
//    printf("%0.10f  %0.10f\n", x1, x2);
    return Vec2(x1, y);
}

float rnd()
{
    return float(std::rand())/RAND_MAX;
}

int main()
{
    // Test for whether samples can possibly fall through the boundaries
    // between two edges.
    //
    // This isn't possible with EdgeEqn, but is with EdgeEqn2.
    for(int j = 0; j < 1000000; ++j)
    {
        const float xOff = 1;
        const float yOff = 2;
        Vec2 a(0.1*rnd(),2+0.001*rnd()), b(-0.1*rnd(),-2+0.001*rnd());
        a += Vec2(xOff, yOff);
        b += Vec2(xOff, yOff);
        EdgeEqn2 e1(a, b);
        EdgeEqn2 e2(b, a);

        Vec2 p = findBdry(e1, yOff + 0.1*rnd(), xOff-1, xOff + 1);

        std::cout << "--\n";
        for(int i = -10; i <= 10; ++i)
        {
            Vec2 p1 = p-Vec2(0.1*FLT_EPSILON*i,0);
            if(!(e1.inside(p1) ^ e2.insideNoBdry(p1)))
            {
                printf("%0.10f %0.10f :  ", p1.x, p1.y);
                std::cout << e1.inside(p1)  << "  "
                          << e2.insideNoBdry(p1) << "\n";
            }
        }
    }

    return 0;
}
