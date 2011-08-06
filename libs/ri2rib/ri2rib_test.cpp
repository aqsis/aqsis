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
            RiDisplay((char*)"blah.tif", (char*)"framebuffer", (char*)"rgb",
                      RI_NULL);
            RiBasis(RiBezierBasis, 3, RiCatmullRomBasis, 1);
            RiPixelFilter(RiGaussianFilter, 2, 2);
            RiProjection((char*)"perspective", RI_NULL);
            RiTranslate(0,0,5);
            RiWorldBegin();
                RtLightHandle h = RiLightSource((char*)"pointlight", RI_NULL);
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
