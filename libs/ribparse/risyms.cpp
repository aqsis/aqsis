// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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

/// \file
///
/// \brief Some standard symbol definitions for the RI
/// \author Paul Gregory

#include "risyms.h"

#include <string.h>

#include <aqsis/ri/ri.h>
#include <aqsis/util/exception.h>

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

} // namespace Aqsis

// vi: set et:
