#include <aqsis/ri/ri.h>

// Simple test of libaqsis_ri2rib

int main()
{
    RiBegin(0);
        RiFrameBegin(0);
            RiBasis(RiCatmullRomBasis, 2, RiHermiteBasis, 3);
        RiFrameEnd();

        RiFrameBegin(1);
            RiBasis(RiBezierBasis, 3, RiCatmullRomBasis, 1);
            RiPixelFilter(RiGaussianFilter, 2, 2);
            RiWorldBegin();
                RtLightHandle h = RiLightSource((char*)"blah_shader", RI_NULL);
                RiIlluminate(h, RI_FALSE);
                RiIlluminate(RtPointer(10), RI_TRUE); // Invalid handle!
                float Cs[] = {1,0,0,  0,1,0,  0,0,1,  2,2,2};
                RiSphere(1, -1, 1, 360, "Cs", Cs, RI_NULL);
                float blah[] = {42};
                RiSphere(2, -2, 2, 360, "float blah", blah, RI_NULL);

                int nvertices[] = {5};
                float P[] = {-1,-1,0,  1,-1,0,  1,1,0,   -1,1,0,  -1,-1,2};
                float width[] = {1, 2, 3};
                RiCurves((char*)"cubic", 1, nvertices,
                         (char*)"nonperiodic",
                         "P", P,
                         "width", width, RI_NULL);
            RiWorldEnd();
        RiFrameEnd();
    RiEnd();
    return 0;
}

// vi: set et:
