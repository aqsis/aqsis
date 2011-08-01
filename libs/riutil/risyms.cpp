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

/// \file
///
/// \brief Some standard symbol definitions for the RI
/// \author Paul Gregory

#include <aqsis/riutil/risyms.h>

#include <string.h>

#include <aqsis/ri/ri.h>
#include <aqsis/util/exception.h>

#include <aqsis/riutil/ribwriter.h>

RtToken RI_FRAMEBUFFER      = tokenCast("framebuffer");
RtToken RI_FILE             = tokenCast("file");
RtToken RI_RGB              = tokenCast("rgb");
RtToken RI_RGBA             = tokenCast("rgba");
RtToken RI_RGBZ             = tokenCast("rgbz");
RtToken RI_RGBAZ            = tokenCast("rgbaz");
RtToken RI_A                = tokenCast("a");
RtToken RI_Z                = tokenCast("z");
RtToken RI_AZ               = tokenCast("az");
RtToken RI_MERGE            = tokenCast("merge");
RtToken RI_ORIGIN           = tokenCast("origin");
RtToken RI_PERSPECTIVE      = tokenCast("perspective");
RtToken RI_ORTHOGRAPHIC     = tokenCast("orthographic");
RtToken RI_HIDDEN           = tokenCast("hidden");
RtToken RI_PAINT            = tokenCast("paint");
RtToken RI_CONSTANT         = tokenCast("constant");
RtToken RI_SMOOTH           = tokenCast("smooth");
RtToken RI_FLATNESS         = tokenCast("flatness");
RtToken RI_FOV              = tokenCast("fov");

RtToken RI_AMBIENTLIGHT     = tokenCast("ambientlight");
RtToken RI_POINTLIGHT       = tokenCast("pointlight");
RtToken RI_DISTANTLIGHT     = tokenCast("distantlight");
RtToken RI_SPOTLIGHT        = tokenCast("spotlight");
RtToken RI_INTENSITY        = tokenCast("intensity");
RtToken RI_LIGHTCOLOR       = tokenCast("lightcolor");
RtToken RI_FROM             = tokenCast("from");
RtToken RI_TO               = tokenCast("to");
RtToken RI_CONEANGLE        = tokenCast("coneangle");
RtToken RI_CONEDELTAANGLE   = tokenCast("conedeltaangle");
RtToken RI_BEAMDISTRIBUTION = tokenCast("beamdistribution");
RtToken RI_MATTE            = tokenCast("matte");
RtToken RI_METAL            = tokenCast("metal");
RtToken RI_PLASTIC          = tokenCast("plastic");
RtToken RI_PAINTEDPLASTIC   = tokenCast("paintedplastic");
RtToken RI_KA               = tokenCast("Ka");
RtToken RI_KD               = tokenCast("Kd");
RtToken RI_KS               = tokenCast("Ks");
RtToken RI_ROUGHNESS        = tokenCast("roughness");
RtToken RI_SPECULARCOLOR    = tokenCast("specularcolor");
RtToken RI_DEPTHCUE         = tokenCast("depthcue");
RtToken RI_FOG              = tokenCast("fog");
RtToken RI_BUMPY            = tokenCast("bumpy");
RtToken RI_MINDISTANCE      = tokenCast("mindistance");
RtToken RI_MAXDISTANCE      = tokenCast("maxdistance");
RtToken RI_BACKGROUND       = tokenCast("background");
RtToken RI_DISTANCE         = tokenCast("distance");

RtToken RI_RASTER           = tokenCast("raster");
RtToken RI_SCREEN           = tokenCast("screen");
RtToken RI_CAMERA           = tokenCast("camera");
RtToken RI_WORLD            = tokenCast("world");
RtToken RI_OBJECT           = tokenCast("object");
RtToken RI_INSIDE           = tokenCast("inside");
RtToken RI_OUTSIDE          = tokenCast("outside");
RtToken RI_LH               = tokenCast("lh");
RtToken RI_RH               = tokenCast("rh");
RtToken RI_P                = tokenCast("P");
RtToken RI_PZ               = tokenCast("Pz");
RtToken RI_PW               = tokenCast("Pw");
RtToken RI_N                = tokenCast("N");
RtToken RI_NP               = tokenCast("Np");
RtToken RI_CS               = tokenCast("Cs");
RtToken RI_OS               = tokenCast("Os");
RtToken RI_S                = tokenCast("s");
RtToken RI_T                = tokenCast("t");
RtToken RI_ST               = tokenCast("st");
RtToken RI_BILINEAR         = tokenCast("bilinear");
RtToken RI_BICUBIC          = tokenCast("bicubic");
RtToken RI_CUBIC            = tokenCast("cubic");
RtToken RI_LINEAR           = tokenCast("linear");
RtToken RI_PRIMITIVE        = tokenCast("primitive");
RtToken RI_INTERSECTION     = tokenCast("intersection");
RtToken RI_UNION            = tokenCast("union");
RtToken RI_DIFFERENCE       = tokenCast("difference");
RtToken RI_WRAP             = tokenCast("wrap");
RtToken RI_NOWRAP           = tokenCast("nowrap");
RtToken RI_PERIODIC         = tokenCast("periodic");
RtToken RI_NONPERIODIC      = tokenCast("nonperiodic");
RtToken RI_CLAMP            = tokenCast("clamp");
RtToken RI_BLACK            = tokenCast("black");
RtToken RI_IGNORE           = tokenCast("ignore");
RtToken RI_PRINT            = tokenCast("print");
RtToken RI_ABORT            = tokenCast("abort");
RtToken RI_HANDLER          = tokenCast("handler");
RtToken RI_IDENTIFIER       = tokenCast("identifier");
RtToken RI_NAME             = tokenCast("name");
RtToken RI_CURRENT          = tokenCast("current");
RtToken RI_SHADER           = tokenCast("shader");
RtToken RI_EYE              = tokenCast("eye");
RtToken RI_NDC              = tokenCast("ndc");
RtToken RI_AMPLITUDE        = tokenCast("amplitude");
RtToken RI_COMMENT          = tokenCast("comment");
RtToken RI_CONSTANTWIDTH    = tokenCast("constantwidth");
RtToken RI_KR               = tokenCast("Kr");
RtToken RI_SHINYMETAL       = tokenCast("shinymetal");
RtToken RI_STRUCTURE        = tokenCast("structure");
RtToken RI_TEXTURENAME      = tokenCast("texturename");
RtToken RI_VERBATIM         = tokenCast("verbatim");
RtToken RI_WIDTH            = tokenCast("width");


RtBasis RiBezierBasis =     {{ -1.0f,  3.0f, -3.0f,  1.0f},
                             {  3.0f, -6.0f,  3.0f,  0.0f},
                             { -3.0f,  3.0f,  0.0f,  0.0f},
                             {  1.0f,  0.0f,  0.0f,  0.0f}};
RtBasis RiBSplineBasis =    {{ -1.0f/6.0f,  0.5f,      -0.5f,      1.0f/6.0f},
                             {  0.5f,       -1.0f,      0.5f,      0.0f},
                             { -0.5f,        0.0f,      0.5f,      0.0f},
                             {  1.0f/6.0f,   2.0f/3.0f, 1.0f/6.0f, 0.0f}};
RtBasis RiCatmullRomBasis = {{ -0.5f,  1.5f, -1.5f,  0.5f},
                             {  1.0f, -2.5f,  2.0f, -0.5f},
                             { -0.5f,  0.0f,  0.5f,  0.0f},
                             {  0.0f,  1.0f,  0.0f,  0.0f}};
RtBasis RiHermiteBasis =    {{  2.0f,  1.0f, -2.0f,  1.0f},
                             { -3.0f, -2.0f,  3.0f, -1.0f},
                             {  0.0f,  1.0f,  0.0f,  0.0f},
                             {  1.0f,  0.0f,  0.0f,  0.0f}};
RtBasis RiPowerBasis =      {{  1.0f,  0.0f,  0.0f,  0.0f},
                             {  0.0f,  1.0f,  0.0f,  0.0f},
                             {  0.0f,  0.0f,  1.0f,  0.0f},
                             {  0.0f,  0.0f,  0.0f,  1.0f}};

namespace Aqsis {

RtFilterFunc getFilterFuncByName(const char* name)
{
    if     (!strcmp(name, "box"))         return &::RiBoxFilter;
    else if(!strcmp(name, "gaussian"))    return &::RiGaussianFilter;
    else if(!strcmp(name, "triangle"))    return &::RiTriangleFilter;
    else if(!strcmp(name, "mitchell"))    return &::RiMitchellFilter;
    else if(!strcmp(name, "catmull-rom")) return &::RiCatmullRomFilter;
    else if(!strcmp(name, "sinc"))        return &::RiSincFilter;
    else if(!strcmp(name, "bessel"))      return &::RiBesselFilter;
    else if(!strcmp(name, "disk"))        return &::RiDiskFilter;
    else
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_BadToken,
            "unknown filter function \"" << name << "\"");
        return 0;
    }
}

RtConstBasis* getBasisByName(const char* name)
{
    if     (!strcmp(name, "bezier"))      return &::RiBezierBasis;
    else if(!strcmp(name, "b-spline"))    return &::RiBSplineBasis;
    else if(!strcmp(name, "catmull-rom")) return &::RiCatmullRomBasis;
    else if(!strcmp(name, "hermite"))     return &::RiHermiteBasis;
    else if(!strcmp(name, "power"))       return &::RiPowerBasis;
    else
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_BadToken,
            "unknown basis \"" << name << "\"");
        return 0;
    }
}

RtErrorFunc getErrorFuncByName(const char* name)
{
    if     (!strcmp(name, "ignore")) return &::RiErrorIgnore;
    else if(!strcmp(name, "print"))  return &::RiErrorPrint;
    else if(!strcmp(name, "abort"))  return &::RiErrorAbort;
    else
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_BadToken,
            "unknown error handler function \"" << name << "\"");
        return 0;
    }
}

RtProcSubdivFunc getProcSubdivFuncByName(const char* name)
{
    if     (!strcmp(name, "DelayedReadArchive")) return &::RiProcDelayedReadArchive;
    else if(!strcmp(name, "RunProgram"))         return &::RiProcRunProgram;
    else if(!strcmp(name, "DynamicLoad"))        return &::RiProcDynamicLoad;
    else
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_BadToken,
                    "unknown procedural function \"" << name << "\"");
        return 0;
    }
}

void registerStdFuncs(RibWriterServices& writer)
{
    writer.registerFilterFunc("box", RiBoxFilter);
    writer.registerFilterFunc("gaussian", RiGaussianFilter);
    writer.registerFilterFunc("triangle", RiTriangleFilter);
    writer.registerFilterFunc("mitchell", RiMitchellFilter);
    writer.registerFilterFunc("catmull-rom", RiCatmullRomFilter);
    writer.registerFilterFunc("sinc", RiSincFilter);
    writer.registerFilterFunc("bessel", RiBesselFilter);
    writer.registerFilterFunc("disk", RiDiskFilter);

    writer.registerProcSubdivFunc("DelayedReadArchive", RiProcDelayedReadArchive);
    writer.registerProcSubdivFunc("RunProgram", RiProcRunProgram);
    writer.registerProcSubdivFunc("DynamicLoad", RiProcDynamicLoad);

    writer.registerErrorFunc("ignore", RiErrorIgnore);
    writer.registerErrorFunc("print", RiErrorPrint);
    writer.registerErrorFunc("abort", RiErrorAbort);
}

} // namespace Aqsis

// vi: set et:
