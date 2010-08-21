#include <aqsis/ri/ri.h>
#include <sstream>
#include <iostream>

// Simple test of libaqsis_ri2rib

int main()
{
    std::ostringstream output;

    // Test setting of options.
    const char* outputType = "Ascii";
    RiOption((char*)"RI2RIB_Output", "Type", &outputType,
                                     "OStream", &output, RI_NULL);
    int indentSize = 2;
    const char* indentType = "Space";
    RiOption((char*)"RI2RIB_Indentation", "Size", &indentSize,
                                          "Type", &indentType, RI_NULL);

    RtPointer badHandle = RtPointer(0xDEADBEEF);

    // Create RIB
    RiBegin(0);
        RiFrameBegin(0);
            RiBasis(RiCatmullRomBasis, 2, RiHermiteBasis, 3);
        RiFrameEnd();

        RiArchiveRecord((char*)"comment",
                        (char*)" Note that we expect a bad handle error "
                               "regarding %p somewhere here!", badHandle);

        RiFrameBegin(1);
            RiBasis(RiBezierBasis, 3, RiCatmullRomBasis, 1);
            RiPixelFilter(RiGaussianFilter, 2, 2);
            RiWorldBegin();
                RtLightHandle h = RiLightSource((char*)"blah_shader", RI_NULL);
                RiIlluminate(h, RI_FALSE);
                RiIlluminate(badHandle, RI_TRUE); // Invalid handle!
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

    // Stream the output buffer to stdout
    std::cout << output.str();
    return 0;
}

// vi: set et:
