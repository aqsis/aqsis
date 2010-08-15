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
/// \brief Converter from plain RI calls to the Ri::Renderer interface.
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///

#include <vector>
#include <stdarg.h>

#include <aqsis/ri/ri.h>
#include <aqsis/util/exception.h>
#include <aqsis/riutil/interpclasscounts.h>
#include "errorhandlerimpl.h"
#include "ricxx.h"
#include "ricxxutil.h"

namespace Aqsis {

// Global context.
extern Ri::Renderer* g_context;
extern Ri::RendererServices* g_services;

}

using namespace Aqsis;
typedef SqInterpClassCounts IClassCounts;

namespace {

std::vector<Ri::Param> g_pList;
std::vector<std::string> g_nameStorage;

// Build a Ri::ParamList from the corresponding C API (count,tokens[],values[])
//
// This function makes use of global storage for the param list - it's _not_
// threadsafe or reentrant!  That shouldn't present a problem though, since
// the C API inherently uses globals...
Ri::ParamList buildParamList(RtInt count, RtToken tokens[], RtPointer values[],
                             const IClassCounts& iclassCounts)
{
    if(count == 0)
        return Ri::ParamList();
    g_pList.clear();
    for(int i = 0; i < count; ++i)
    {
        const char* nameBegin = 0;
        const char* nameEnd = 0;
        Ri::TypeSpec spec = g_services->getDeclaration(tokens[i], &nameBegin,
                                                       &nameEnd);
        if(*nameEnd != '\0')
        {
            // Usually we don't have to allocate space for the name,
            // but if someone has put whitespace after it in the type
            // string then we do:
            g_nameStorage.push_back(std::string(nameBegin, nameEnd));
            nameBegin = g_nameStorage.back().c_str();
        }
        int size = iclassCount(iclassCounts, spec.iclass) *
                    spec.storageCount();
        g_pList.push_back(Ri::Param(spec, nameBegin, values[i], size));
    }
    return Ri::ParamList(&g_pList[0], count);
}

} // anon namespace

#define EXCEPTION_TRY_GUARD try {

// Exception catch guard to prevent exceptions propagating outside of Ri
// calls.  This could be dumped directly into the generated code, but it
// clutters it up a lot.
#define EXCEPTION_CATCH_GUARD(procName)                                     \
}                                                                           \
catch(const XqValidation& e)                                                \
{                                                                           \
    AQSIS_LOG_ERROR(g_services->errorHandler(), e.code())                   \
        << "ignoring invalid " procName ": " << e.what();                   \
}                                                                           \
catch(const XqException& e)                                                 \
{                                                                           \
    AQSIS_LOG_ERROR(g_services->errorHandler(), e.code()) << e.what();      \
}                                                                           \
catch(const std::exception& e)                                              \
{                                                                           \
    AQSIS_LOG_SEVERE(g_services->errorHandler(), EqE_Bug)                   \
        << "std::exception encountered in " procName ": " << e.what();      \
}                                                                           \
catch(...)                                                                  \
{                                                                           \
    AQSIS_LOG_SEVERE(g_services->errorHandler(), EqE_Bug)                   \
        << "unknown exception encountered in " procName;                    \
}

/*

--------------------------------------------------------------------------------
C RI functions, forwarding to the C++ context object.

We implement all the C interface functions as wrappers around the internal C++
API.  These convert parameters from the C API to the C++ one, and call through
to the current C++ context object.  Any exceptions are caught at the C API
boundary and passed through to the current error handler.

Most of the complicated stuff here is in computing appropriate array lengths
from the C API, which uses implicit array lengths everywhere.
--------------------------------------------------------------------------------

TODO: RiCurvesV and RiPointsV IClassLengths are wrong!!

[[[cog

from codegenutils import *
from Cheetah.Template import Template
riXml = parseXmlTree(riXmlPath)

# FIXME: Use correct basisU/V steps below!
iclassCountSnippets = {
    'PatchMesh': 'iclassCounts = patchMeshIClassCounts(type, nu, uwrap, nv, vwrap, 3, 3);',
    'Curves': 'iclassCounts = curvesIClassCounts(type, nvertices, wrap, 3);',
    'Geometry': ''
}

procTemplate = '''
extern "C"
$returnType ${cProcName}($formals)
{
    EXCEPTION_TRY_GUARD
#for $arg in $arrayArgs
    Ri::${arg.findtext('Type')[2:]} ${arg.findtext('Name')}(${arg.findtext('Name')}_in, $arrayLen(arg));
#end for
#if $proc.haschild('Arguments/ParamList')
    IClassCounts iclassCounts(1,1,1,1,1);
 #set $icLen = $proc.find('IClassLengths')
 #if $icLen is not None
  #if $icLen.haschild('ComplicatedCustomImpl')
    $iclassCountSnippets[$procName]
  #else
   #if $icLen.haschild('Uniform')
    iclassCounts.uniform = $icLen.findtext('Uniform');
   #end if
   #if $icLen.haschild('Varying')
    #if $icLen.findtext('Varying') == 'countP(pList)'
    ## special case for RiPolygon and RiPoints
    #assert $args[0].findtext('RibValue') == 'countP(pList)'
    iclassCounts.varying = $args[0].findtext('Name');
    #else
    iclassCounts.varying = $icLen.findtext('Varying');
    #end if
   #end if
   #if $icLen.haschild('Vertex')
    iclassCounts.vertex = $icLen.findtext('Vertex');
   #else
    iclassCounts.vertex = iclassCounts.varying;
   #end if
   #if $icLen.haschild('FaceVarying')
    iclassCounts.facevarying = $icLen.findtext('FaceVarying');
   #else
    iclassCounts.facevarying = iclassCounts.varying;
   #end if
   #if $icLen.haschild('FaceVertex')
    iclassCounts.facevertex = $icLen.findtext('FaceVertex');
   #else
    iclassCounts.facevertex = iclassCounts.facevarying;
   #end if
  #end if
 #end if
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
#end if
    return g_context->${procName}($callArgs);
    EXCEPTION_CATCH_GUARD("$procName");
#if $returnType != 'RtVoid'
    return 0;
#end if
}

'''

# Get the length expression for required array arguments
def arrayLen(arg):
    if arg.haschild('Length'):
        return arg.findtext('Length')
    else:
        return arg.findtext('RiLength')

for proc in riXml.findall('Procedures/Procedure'):
    if proc.haschild('Rib'):
        procName = proc.findtext('Name')
        args = proc.findall('Arguments/Argument')
        formals = [formalArgC(arg, arraySuffix='_in') for arg in args]
        callArgsXml = [a for a in args if not a.haschild('RibValue')]
        arrayArgs = [a for a in callArgsXml if
                     a.findtext('Type').endswith('Array')]
        # Ugh!  The length of 'knot' in trimcurve depends on 'n', so here's a
        # special case to swap the order.
        if procName == 'TrimCurve':
            arrayArgs[2], arrayArgs[5] = arrayArgs[5], arrayArgs[2]
        callArgs = [a.findtext('Name') for a in callArgsXml]
        returnType = proc.findtext('ReturnType')
        cProcName = cName(proc)
        if proc.haschild('Arguments/ParamList'):
            cProcName = cProcName + 'V'
            callArgs.append('pList')
            formals += ['RtInt count', 'RtToken tokens[]','RtPointer values[]']
        callArgs = ', '.join(callArgs)
        formals = ', '.join(formals)
        cog.out(str(Template(procTemplate, searchList=locals())))

]]]*/

extern "C"
RtToken RiDeclare(RtString name, RtString declaration)
{
    EXCEPTION_TRY_GUARD
    return g_context->Declare(name, declaration);
    EXCEPTION_CATCH_GUARD("Declare");
    return 0;
}


extern "C"
RtVoid RiFrameBegin(RtInt number)
{
    EXCEPTION_TRY_GUARD
    return g_context->FrameBegin(number);
    EXCEPTION_CATCH_GUARD("FrameBegin");
}


extern "C"
RtVoid RiFrameEnd()
{
    EXCEPTION_TRY_GUARD
    return g_context->FrameEnd();
    EXCEPTION_CATCH_GUARD("FrameEnd");
}


extern "C"
RtVoid RiWorldBegin()
{
    EXCEPTION_TRY_GUARD
    return g_context->WorldBegin();
    EXCEPTION_CATCH_GUARD("WorldBegin");
}


extern "C"
RtVoid RiWorldEnd()
{
    EXCEPTION_TRY_GUARD
    return g_context->WorldEnd();
    EXCEPTION_CATCH_GUARD("WorldEnd");
}


extern "C"
RtVoid RiIfBegin(RtString condition)
{
    EXCEPTION_TRY_GUARD
    return g_context->IfBegin(condition);
    EXCEPTION_CATCH_GUARD("IfBegin");
}


extern "C"
RtVoid RiElseIf(RtString condition)
{
    EXCEPTION_TRY_GUARD
    return g_context->ElseIf(condition);
    EXCEPTION_CATCH_GUARD("ElseIf");
}


extern "C"
RtVoid RiElse()
{
    EXCEPTION_TRY_GUARD
    return g_context->Else();
    EXCEPTION_CATCH_GUARD("Else");
}


extern "C"
RtVoid RiIfEnd()
{
    EXCEPTION_TRY_GUARD
    return g_context->IfEnd();
    EXCEPTION_CATCH_GUARD("IfEnd");
}


extern "C"
RtVoid RiFormat(RtInt xresolution, RtInt yresolution, RtFloat pixelaspectratio)
{
    EXCEPTION_TRY_GUARD
    return g_context->Format(xresolution, yresolution, pixelaspectratio);
    EXCEPTION_CATCH_GUARD("Format");
}


extern "C"
RtVoid RiFrameAspectRatio(RtFloat frameratio)
{
    EXCEPTION_TRY_GUARD
    return g_context->FrameAspectRatio(frameratio);
    EXCEPTION_CATCH_GUARD("FrameAspectRatio");
}


extern "C"
RtVoid RiScreenWindow(RtFloat left, RtFloat right, RtFloat bottom, RtFloat top)
{
    EXCEPTION_TRY_GUARD
    return g_context->ScreenWindow(left, right, bottom, top);
    EXCEPTION_CATCH_GUARD("ScreenWindow");
}


extern "C"
RtVoid RiCropWindow(RtFloat xmin, RtFloat xmax, RtFloat ymin, RtFloat ymax)
{
    EXCEPTION_TRY_GUARD
    return g_context->CropWindow(xmin, xmax, ymin, ymax);
    EXCEPTION_CATCH_GUARD("CropWindow");
}


extern "C"
RtVoid RiProjectionV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Projection(name, pList);
    EXCEPTION_CATCH_GUARD("Projection");
}


extern "C"
RtVoid RiClipping(RtFloat cnear, RtFloat cfar)
{
    EXCEPTION_TRY_GUARD
    return g_context->Clipping(cnear, cfar);
    EXCEPTION_CATCH_GUARD("Clipping");
}


extern "C"
RtVoid RiClippingPlane(RtFloat x, RtFloat y, RtFloat z, RtFloat nx, RtFloat ny, RtFloat nz)
{
    EXCEPTION_TRY_GUARD
    return g_context->ClippingPlane(x, y, z, nx, ny, nz);
    EXCEPTION_CATCH_GUARD("ClippingPlane");
}


extern "C"
RtVoid RiDepthOfField(RtFloat fstop, RtFloat focallength, RtFloat focaldistance)
{
    EXCEPTION_TRY_GUARD
    return g_context->DepthOfField(fstop, focallength, focaldistance);
    EXCEPTION_CATCH_GUARD("DepthOfField");
}


extern "C"
RtVoid RiShutter(RtFloat opentime, RtFloat closetime)
{
    EXCEPTION_TRY_GUARD
    return g_context->Shutter(opentime, closetime);
    EXCEPTION_CATCH_GUARD("Shutter");
}


extern "C"
RtVoid RiPixelVariance(RtFloat variance)
{
    EXCEPTION_TRY_GUARD
    return g_context->PixelVariance(variance);
    EXCEPTION_CATCH_GUARD("PixelVariance");
}


extern "C"
RtVoid RiPixelSamples(RtFloat xsamples, RtFloat ysamples)
{
    EXCEPTION_TRY_GUARD
    return g_context->PixelSamples(xsamples, ysamples);
    EXCEPTION_CATCH_GUARD("PixelSamples");
}


extern "C"
RtVoid RiPixelFilter(RtFilterFunc function, RtFloat xwidth, RtFloat ywidth)
{
    EXCEPTION_TRY_GUARD
    return g_context->PixelFilter(function, xwidth, ywidth);
    EXCEPTION_CATCH_GUARD("PixelFilter");
}


extern "C"
RtVoid RiExposure(RtFloat gain, RtFloat gamma)
{
    EXCEPTION_TRY_GUARD
    return g_context->Exposure(gain, gamma);
    EXCEPTION_CATCH_GUARD("Exposure");
}


extern "C"
RtVoid RiImagerV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Imager(name, pList);
    EXCEPTION_CATCH_GUARD("Imager");
}


extern "C"
RtVoid RiQuantize(RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ditheramplitude)
{
    EXCEPTION_TRY_GUARD
    return g_context->Quantize(type, one, min, max, ditheramplitude);
    EXCEPTION_CATCH_GUARD("Quantize");
}


extern "C"
RtVoid RiDisplayV(RtToken name, RtToken type, RtToken mode, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Display(name, type, mode, pList);
    EXCEPTION_CATCH_GUARD("Display");
}


extern "C"
RtVoid RiHiderV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Hider(name, pList);
    EXCEPTION_CATCH_GUARD("Hider");
}


extern "C"
RtVoid RiColorSamples(RtInt N, RtFloat nRGB_in[], RtFloat RGBn_in[])
{
    EXCEPTION_TRY_GUARD
    Ri::FloatArray nRGB(nRGB_in, 3*N);
    Ri::FloatArray RGBn(RGBn_in, size(nRGB));
    return g_context->ColorSamples(nRGB, RGBn);
    EXCEPTION_CATCH_GUARD("ColorSamples");
}


extern "C"
RtVoid RiRelativeDetail(RtFloat relativedetail)
{
    EXCEPTION_TRY_GUARD
    return g_context->RelativeDetail(relativedetail);
    EXCEPTION_CATCH_GUARD("RelativeDetail");
}


extern "C"
RtVoid RiOptionV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Option(name, pList);
    EXCEPTION_CATCH_GUARD("Option");
}


extern "C"
RtVoid RiAttributeBegin()
{
    EXCEPTION_TRY_GUARD
    return g_context->AttributeBegin();
    EXCEPTION_CATCH_GUARD("AttributeBegin");
}


extern "C"
RtVoid RiAttributeEnd()
{
    EXCEPTION_TRY_GUARD
    return g_context->AttributeEnd();
    EXCEPTION_CATCH_GUARD("AttributeEnd");
}


extern "C"
RtVoid RiColor(RtColor Cq)
{
    EXCEPTION_TRY_GUARD
    return g_context->Color(Cq);
    EXCEPTION_CATCH_GUARD("Color");
}


extern "C"
RtVoid RiOpacity(RtColor Os)
{
    EXCEPTION_TRY_GUARD
    return g_context->Opacity(Os);
    EXCEPTION_CATCH_GUARD("Opacity");
}


extern "C"
RtVoid RiTextureCoordinates(RtFloat s1, RtFloat t1, RtFloat s2, RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4, RtFloat t4)
{
    EXCEPTION_TRY_GUARD
    return g_context->TextureCoordinates(s1, t1, s2, t2, s3, t3, s4, t4);
    EXCEPTION_CATCH_GUARD("TextureCoordinates");
}


extern "C"
RtLightHandle RiLightSourceV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->LightSource(name, pList);
    EXCEPTION_CATCH_GUARD("LightSource");
    return 0;
}


extern "C"
RtLightHandle RiAreaLightSourceV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->AreaLightSource(name, pList);
    EXCEPTION_CATCH_GUARD("AreaLightSource");
    return 0;
}


extern "C"
RtVoid RiIlluminate(RtLightHandle light, RtBoolean onoff)
{
    EXCEPTION_TRY_GUARD
    return g_context->Illuminate(light, onoff);
    EXCEPTION_CATCH_GUARD("Illuminate");
}


extern "C"
RtVoid RiSurfaceV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Surface(name, pList);
    EXCEPTION_CATCH_GUARD("Surface");
}


extern "C"
RtVoid RiDisplacementV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Displacement(name, pList);
    EXCEPTION_CATCH_GUARD("Displacement");
}


extern "C"
RtVoid RiAtmosphereV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Atmosphere(name, pList);
    EXCEPTION_CATCH_GUARD("Atmosphere");
}


extern "C"
RtVoid RiInteriorV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Interior(name, pList);
    EXCEPTION_CATCH_GUARD("Interior");
}


extern "C"
RtVoid RiExteriorV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Exterior(name, pList);
    EXCEPTION_CATCH_GUARD("Exterior");
}


extern "C"
RtVoid RiShaderLayerV(RtToken type, RtToken name, RtToken layername, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->ShaderLayer(type, name, layername, pList);
    EXCEPTION_CATCH_GUARD("ShaderLayer");
}


extern "C"
RtVoid RiConnectShaderLayers(RtToken type, RtToken layer1, RtToken variable1, RtToken layer2, RtToken variable2)
{
    EXCEPTION_TRY_GUARD
    return g_context->ConnectShaderLayers(type, layer1, variable1, layer2, variable2);
    EXCEPTION_CATCH_GUARD("ConnectShaderLayers");
}


extern "C"
RtVoid RiShadingRate(RtFloat size)
{
    EXCEPTION_TRY_GUARD
    return g_context->ShadingRate(size);
    EXCEPTION_CATCH_GUARD("ShadingRate");
}


extern "C"
RtVoid RiShadingInterpolation(RtToken type)
{
    EXCEPTION_TRY_GUARD
    return g_context->ShadingInterpolation(type);
    EXCEPTION_CATCH_GUARD("ShadingInterpolation");
}


extern "C"
RtVoid RiMatte(RtBoolean onoff)
{
    EXCEPTION_TRY_GUARD
    return g_context->Matte(onoff);
    EXCEPTION_CATCH_GUARD("Matte");
}


extern "C"
RtVoid RiBound(RtBound bound)
{
    EXCEPTION_TRY_GUARD
    return g_context->Bound(bound);
    EXCEPTION_CATCH_GUARD("Bound");
}


extern "C"
RtVoid RiDetail(RtBound bound)
{
    EXCEPTION_TRY_GUARD
    return g_context->Detail(bound);
    EXCEPTION_CATCH_GUARD("Detail");
}


extern "C"
RtVoid RiDetailRange(RtFloat offlow, RtFloat onlow, RtFloat onhigh, RtFloat offhigh)
{
    EXCEPTION_TRY_GUARD
    return g_context->DetailRange(offlow, onlow, onhigh, offhigh);
    EXCEPTION_CATCH_GUARD("DetailRange");
}


extern "C"
RtVoid RiGeometricApproximation(RtToken type, RtFloat value)
{
    EXCEPTION_TRY_GUARD
    return g_context->GeometricApproximation(type, value);
    EXCEPTION_CATCH_GUARD("GeometricApproximation");
}


extern "C"
RtVoid RiOrientation(RtToken orientation)
{
    EXCEPTION_TRY_GUARD
    return g_context->Orientation(orientation);
    EXCEPTION_CATCH_GUARD("Orientation");
}


extern "C"
RtVoid RiReverseOrientation()
{
    EXCEPTION_TRY_GUARD
    return g_context->ReverseOrientation();
    EXCEPTION_CATCH_GUARD("ReverseOrientation");
}


extern "C"
RtVoid RiSides(RtInt nsides)
{
    EXCEPTION_TRY_GUARD
    return g_context->Sides(nsides);
    EXCEPTION_CATCH_GUARD("Sides");
}


extern "C"
RtVoid RiIdentity()
{
    EXCEPTION_TRY_GUARD
    return g_context->Identity();
    EXCEPTION_CATCH_GUARD("Identity");
}


extern "C"
RtVoid RiTransform(RtMatrix transform)
{
    EXCEPTION_TRY_GUARD
    return g_context->Transform(transform);
    EXCEPTION_CATCH_GUARD("Transform");
}


extern "C"
RtVoid RiConcatTransform(RtMatrix transform)
{
    EXCEPTION_TRY_GUARD
    return g_context->ConcatTransform(transform);
    EXCEPTION_CATCH_GUARD("ConcatTransform");
}


extern "C"
RtVoid RiPerspective(RtFloat fov)
{
    EXCEPTION_TRY_GUARD
    return g_context->Perspective(fov);
    EXCEPTION_CATCH_GUARD("Perspective");
}


extern "C"
RtVoid RiTranslate(RtFloat dx, RtFloat dy, RtFloat dz)
{
    EXCEPTION_TRY_GUARD
    return g_context->Translate(dx, dy, dz);
    EXCEPTION_CATCH_GUARD("Translate");
}


extern "C"
RtVoid RiRotate(RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz)
{
    EXCEPTION_TRY_GUARD
    return g_context->Rotate(angle, dx, dy, dz);
    EXCEPTION_CATCH_GUARD("Rotate");
}


extern "C"
RtVoid RiScale(RtFloat sx, RtFloat sy, RtFloat sz)
{
    EXCEPTION_TRY_GUARD
    return g_context->Scale(sx, sy, sz);
    EXCEPTION_CATCH_GUARD("Scale");
}


extern "C"
RtVoid RiSkew(RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1, RtFloat dx2, RtFloat dy2, RtFloat dz2)
{
    EXCEPTION_TRY_GUARD
    return g_context->Skew(angle, dx1, dy1, dz1, dx2, dy2, dz2);
    EXCEPTION_CATCH_GUARD("Skew");
}


extern "C"
RtVoid RiCoordinateSystem(RtToken space)
{
    EXCEPTION_TRY_GUARD
    return g_context->CoordinateSystem(space);
    EXCEPTION_CATCH_GUARD("CoordinateSystem");
}


extern "C"
RtVoid RiCoordSysTransform(RtToken space)
{
    EXCEPTION_TRY_GUARD
    return g_context->CoordSysTransform(space);
    EXCEPTION_CATCH_GUARD("CoordSysTransform");
}


extern "C"
RtVoid RiTransformBegin()
{
    EXCEPTION_TRY_GUARD
    return g_context->TransformBegin();
    EXCEPTION_CATCH_GUARD("TransformBegin");
}


extern "C"
RtVoid RiTransformEnd()
{
    EXCEPTION_TRY_GUARD
    return g_context->TransformEnd();
    EXCEPTION_CATCH_GUARD("TransformEnd");
}


extern "C"
RtVoid RiResourceV(RtToken handle, RtToken type, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Resource(handle, type, pList);
    EXCEPTION_CATCH_GUARD("Resource");
}


extern "C"
RtVoid RiResourceBegin()
{
    EXCEPTION_TRY_GUARD
    return g_context->ResourceBegin();
    EXCEPTION_CATCH_GUARD("ResourceBegin");
}


extern "C"
RtVoid RiResourceEnd()
{
    EXCEPTION_TRY_GUARD
    return g_context->ResourceEnd();
    EXCEPTION_CATCH_GUARD("ResourceEnd");
}


extern "C"
RtVoid RiAttributeV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Attribute(name, pList);
    EXCEPTION_CATCH_GUARD("Attribute");
}


extern "C"
RtVoid RiPolygonV(RtInt nvertices, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = nvertices;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Polygon(pList);
    EXCEPTION_CATCH_GUARD("Polygon");
}


extern "C"
RtVoid RiGeneralPolygonV(RtInt nloops, RtInt nverts_in[], RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    Ri::IntArray nverts(nverts_in, nloops);
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = sum(nverts);
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->GeneralPolygon(nverts, pList);
    EXCEPTION_CATCH_GUARD("GeneralPolygon");
}


extern "C"
RtVoid RiPointsPolygonsV(RtInt npolys, RtInt nverts_in[], RtInt verts_in[], RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    Ri::IntArray nverts(nverts_in, npolys);
    Ri::IntArray verts(verts_in, sum(nverts));
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.uniform = size(nverts);
    iclassCounts.varying = max(verts)+1;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = sum(nverts);
    iclassCounts.facevertex = iclassCounts.facevarying;
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->PointsPolygons(nverts, verts, pList);
    EXCEPTION_CATCH_GUARD("PointsPolygons");
}


extern "C"
RtVoid RiPointsGeneralPolygonsV(RtInt npolys, RtInt nloops_in[], RtInt nverts_in[], RtInt verts_in[], RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    Ri::IntArray nloops(nloops_in, npolys);
    Ri::IntArray nverts(nverts_in, sum(nloops));
    Ri::IntArray verts(verts_in, sum(nverts));
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.uniform = size(nloops);
    iclassCounts.varying = max(verts)+1;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = sum(nverts);
    iclassCounts.facevertex = iclassCounts.facevarying;
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->PointsGeneralPolygons(nloops, nverts, verts, pList);
    EXCEPTION_CATCH_GUARD("PointsGeneralPolygons");
}


extern "C"
RtVoid RiBasis(RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep)
{
    EXCEPTION_TRY_GUARD
    return g_context->Basis(ubasis, ustep, vbasis, vstep);
    EXCEPTION_CATCH_GUARD("Basis");
}


extern "C"
RtVoid RiPatchV(RtToken type, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = 4;
    iclassCounts.vertex = strcmp(type,"bilinear")==0 ? 4 : 16;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Patch(type, pList);
    EXCEPTION_CATCH_GUARD("Patch");
}


extern "C"
RtVoid RiPatchMeshV(RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts = patchMeshIClassCounts(type, nu, uwrap, nv, vwrap, 3, 3);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->PatchMesh(type, nu, uwrap, nv, vwrap, pList);
    EXCEPTION_CATCH_GUARD("PatchMesh");
}


extern "C"
RtVoid RiNuPatchV(RtInt nu, RtInt uorder, RtFloat uknot_in[], RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder, RtFloat vknot_in[], RtFloat vmin, RtFloat vmax, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    Ri::FloatArray uknot(uknot_in, nu+uorder);
    Ri::FloatArray vknot(vknot_in, nv+vorder);
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.uniform = (1+nu-uorder+1)*(1+nv-vorder+1);
    iclassCounts.varying = (1+nu-uorder+1)*(1+nv-vorder+1);
    iclassCounts.vertex = nu*nv;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->NuPatch(nu, uorder, uknot, umin, umax, nv, vorder, vknot, vmin, vmax, pList);
    EXCEPTION_CATCH_GUARD("NuPatch");
}


extern "C"
RtVoid RiTrimCurve(RtInt nloops, RtInt ncurves_in[], RtInt order_in[], RtFloat knot_in[], RtFloat min_in[], RtFloat max_in[], RtInt n_in[], RtFloat u_in[], RtFloat v_in[], RtFloat w_in[])
{
    EXCEPTION_TRY_GUARD
    Ri::IntArray ncurves(ncurves_in, nloops);
    Ri::IntArray order(order_in, sum(ncurves));
    Ri::IntArray n(n_in, size(order));
    Ri::FloatArray min(min_in, size(order));
    Ri::FloatArray max(max_in, size(order));
    Ri::FloatArray knot(knot_in, sum(order)+sum(n));
    Ri::FloatArray u(u_in, sum(n));
    Ri::FloatArray v(v_in, size(u));
    Ri::FloatArray w(w_in, size(u));
    return g_context->TrimCurve(ncurves, order, knot, min, max, n, u, v, w);
    EXCEPTION_CATCH_GUARD("TrimCurve");
}


extern "C"
RtVoid RiSubdivisionMeshV(RtToken scheme, RtInt nfaces, RtInt nvertices_in[], RtInt vertices_in[], RtInt ntags, RtToken tags_in[], RtInt nargs_in[], RtInt intargs_in[], RtFloat floatargs_in[], RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    Ri::IntArray nvertices(nvertices_in, nfaces);
    Ri::IntArray vertices(vertices_in, sum(nvertices));
    Ri::TokenArray tags(tags_in, ntags);
    Ri::IntArray nargs(nargs_in, 2*size(tags));
    Ri::IntArray intargs(intargs_in, sum(nargs,0,2));
    Ri::FloatArray floatargs(floatargs_in, sum(nargs,1,2));
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.uniform = size(nvertices);
    iclassCounts.varying = max(vertices)+1;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = sum(nvertices);
    iclassCounts.facevertex = iclassCounts.facevarying;
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->SubdivisionMesh(scheme, nvertices, vertices, tags, nargs, intargs, floatargs, pList);
    EXCEPTION_CATCH_GUARD("SubdivisionMesh");
}


extern "C"
RtVoid RiSphereV(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = 4;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Sphere(radius, zmin, zmax, thetamax, pList);
    EXCEPTION_CATCH_GUARD("Sphere");
}


extern "C"
RtVoid RiConeV(RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = 4;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Cone(height, radius, thetamax, pList);
    EXCEPTION_CATCH_GUARD("Cone");
}


extern "C"
RtVoid RiCylinderV(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = 4;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Cylinder(radius, zmin, zmax, thetamax, pList);
    EXCEPTION_CATCH_GUARD("Cylinder");
}


extern "C"
RtVoid RiHyperboloidV(RtPoint point1, RtPoint point2, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = 4;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Hyperboloid(point1, point2, thetamax, pList);
    EXCEPTION_CATCH_GUARD("Hyperboloid");
}


extern "C"
RtVoid RiParaboloidV(RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = 4;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Paraboloid(rmax, zmin, zmax, thetamax, pList);
    EXCEPTION_CATCH_GUARD("Paraboloid");
}


extern "C"
RtVoid RiDiskV(RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = 4;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Disk(height, radius, thetamax, pList);
    EXCEPTION_CATCH_GUARD("Disk");
}


extern "C"
RtVoid RiTorusV(RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = 4;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Torus(majorrad, minorrad, phimin, phimax, thetamax, pList);
    EXCEPTION_CATCH_GUARD("Torus");
}


extern "C"
RtVoid RiPointsV(RtInt npoints, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = npoints;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Points(pList);
    EXCEPTION_CATCH_GUARD("Points");
}


extern "C"
RtVoid RiCurvesV(RtToken type, RtInt ncurves, RtInt nvertices_in[], RtToken wrap, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    Ri::IntArray nvertices(nvertices_in, ncurves);
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts = curvesIClassCounts(type, nvertices, wrap, 3);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Curves(type, nvertices, wrap, pList);
    EXCEPTION_CATCH_GUARD("Curves");
}


extern "C"
RtVoid RiBlobbyV(RtInt nleaf, RtInt ncode, RtInt code_in[], RtInt nfloats, RtFloat floats_in[], RtInt nstrings, RtToken strings_in[], RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    Ri::IntArray code(code_in, ncode);
    Ri::FloatArray floats(floats_in, nfloats);
    Ri::TokenArray strings(strings_in, nstrings);
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = nleaf;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Blobby(nleaf, code, floats, strings, pList);
    EXCEPTION_CATCH_GUARD("Blobby");
}


extern "C"
RtVoid RiProcedural(RtPointer data, RtBound bound, RtProcSubdivFunc refineproc, RtProcFreeFunc freeproc)
{
    EXCEPTION_TRY_GUARD
    return g_context->Procedural(data, bound, refineproc, freeproc);
    EXCEPTION_CATCH_GUARD("Procedural");
}


extern "C"
RtVoid RiGeometryV(RtToken type, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->Geometry(type, pList);
    EXCEPTION_CATCH_GUARD("Geometry");
}


extern "C"
RtVoid RiSolidBegin(RtToken type)
{
    EXCEPTION_TRY_GUARD
    return g_context->SolidBegin(type);
    EXCEPTION_CATCH_GUARD("SolidBegin");
}


extern "C"
RtVoid RiSolidEnd()
{
    EXCEPTION_TRY_GUARD
    return g_context->SolidEnd();
    EXCEPTION_CATCH_GUARD("SolidEnd");
}


extern "C"
RtObjectHandle RiObjectBegin()
{
    EXCEPTION_TRY_GUARD
    return g_context->ObjectBegin();
    EXCEPTION_CATCH_GUARD("ObjectBegin");
    return 0;
}


extern "C"
RtVoid RiObjectEnd()
{
    EXCEPTION_TRY_GUARD
    return g_context->ObjectEnd();
    EXCEPTION_CATCH_GUARD("ObjectEnd");
}


extern "C"
RtVoid RiObjectInstance(RtObjectHandle handle)
{
    EXCEPTION_TRY_GUARD
    return g_context->ObjectInstance(handle);
    EXCEPTION_CATCH_GUARD("ObjectInstance");
}


extern "C"
RtVoid RiMotionBeginV(RtInt N, RtFloat times_in[])
{
    EXCEPTION_TRY_GUARD
    Ri::FloatArray times(times_in, N);
    return g_context->MotionBegin(times);
    EXCEPTION_CATCH_GUARD("MotionBegin");
}


extern "C"
RtVoid RiMotionEnd()
{
    EXCEPTION_TRY_GUARD
    return g_context->MotionEnd();
    EXCEPTION_CATCH_GUARD("MotionEnd");
}


extern "C"
RtVoid RiMakeTextureV(RtString imagefile, RtString texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->MakeTexture(imagefile, texturefile, swrap, twrap, filterfunc, swidth, twidth, pList);
    EXCEPTION_CATCH_GUARD("MakeTexture");
}


extern "C"
RtVoid RiMakeLatLongEnvironmentV(RtString imagefile, RtString reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->MakeLatLongEnvironment(imagefile, reflfile, filterfunc, swidth, twidth, pList);
    EXCEPTION_CATCH_GUARD("MakeLatLongEnvironment");
}


extern "C"
RtVoid RiMakeCubeFaceEnvironmentV(RtString px, RtString nx, RtString py, RtString ny, RtString pz, RtString nz, RtString reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->MakeCubeFaceEnvironment(px, nx, py, ny, pz, nz, reflfile, fov, filterfunc, swidth, twidth, pList);
    EXCEPTION_CATCH_GUARD("MakeCubeFaceEnvironment");
}


extern "C"
RtVoid RiMakeShadowV(RtString picfile, RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->MakeShadow(picfile, shadowfile, pList);
    EXCEPTION_CATCH_GUARD("MakeShadow");
}


extern "C"
RtVoid RiMakeOcclusionV(RtInt npics, RtString picfiles_in[], RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    Ri::StringArray picfiles(picfiles_in, npics);
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->MakeOcclusion(picfiles, shadowfile, pList);
    EXCEPTION_CATCH_GUARD("MakeOcclusion");
}


extern "C"
RtVoid RiErrorHandler(RtErrorFunc handler)
{
    EXCEPTION_TRY_GUARD
    return g_context->ErrorHandler(handler);
    EXCEPTION_CATCH_GUARD("ErrorHandler");
}


extern "C"
RtVoid RiReadArchiveV(RtToken name, RtArchiveCallback callback, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    return g_context->ReadArchive(name, callback, pList);
    EXCEPTION_CATCH_GUARD("ReadArchive");
}

///[[[end]]]
// End main generated code
//-----------------------------------------------------------------------------

namespace {

std::vector<RtToken> g_tokens;
std::vector<RtPointer> g_values;

// Build a parameter list from a va_list.
//
// This function makes use of global storage for the tokens and values - it's
// _not_ threadsafe or reentrant!  That shouldn't present a problem though,
// since the C API inherently uses globals
void buildParamList(va_list args, RtInt& count,
                    RtToken* &tokens, RtPointer* &values)
{
    count = 0;
    g_tokens.clear();
    g_values.clear();
    RtToken token = va_arg(args, RtToken);
    while(token)
    {
        g_tokens.push_back(token);
        g_values.push_back(va_arg(args, RtPointer));
        token = va_arg(args, RtToken);
        ++count;
    }
    if(count == 0)
    {
        tokens = 0;
        values = 0;
    }
    else
    {
        tokens = &g_tokens[0];
        values = &g_values[0];
    }
}

} // anon. namespace


/*
--------------------------------------------------------------------------------
Code generator for varargs function calls.

Each API function which takes a parameter list has a varargs form.  For
simplicity, we implement all of these as wrappers around the corresponding
'Ri*V' versions, above.
--------------------------------------------------------------------------------

'
[[[cog

varargsProcTemplate = '''
extern "C"
$returnType ${procName}($formals, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, ${args[-1].findtext('Name')});
    buildParamList(args, count, tokens, values);
    va_end(args);

    return ${procName}V($callArgs, count, tokens, values);
}

'''

for proc in riXml.findall('Procedures/Procedure'):
    if proc.haschild('Rib') and proc.haschild('Arguments/ParamList'):
        args = proc.findall('Arguments/Argument')
        formals = ', '.join([formalArgC(arg) for arg in args])
        callArgs = ', '.join([arg.findtext('Name') for arg in args])
        returnType = proc.findtext('ReturnType')
        procName = cName(proc)
        cog.out(str(Template(varargsProcTemplate, searchList=locals())))


]]]*/

extern "C"
RtVoid RiProjection(RtToken name, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, name);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiProjectionV(name, count, tokens, values);
}


extern "C"
RtVoid RiImager(RtToken name, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, name);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiImagerV(name, count, tokens, values);
}


extern "C"
RtVoid RiDisplay(RtToken name, RtToken type, RtToken mode, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, mode);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiDisplayV(name, type, mode, count, tokens, values);
}


extern "C"
RtVoid RiHider(RtToken name, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, name);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiHiderV(name, count, tokens, values);
}


extern "C"
RtVoid RiOption(RtToken name, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, name);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiOptionV(name, count, tokens, values);
}


extern "C"
RtLightHandle RiLightSource(RtToken name, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, name);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiLightSourceV(name, count, tokens, values);
}


extern "C"
RtLightHandle RiAreaLightSource(RtToken name, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, name);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiAreaLightSourceV(name, count, tokens, values);
}


extern "C"
RtVoid RiSurface(RtToken name, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, name);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiSurfaceV(name, count, tokens, values);
}


extern "C"
RtVoid RiDisplacement(RtToken name, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, name);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiDisplacementV(name, count, tokens, values);
}


extern "C"
RtVoid RiAtmosphere(RtToken name, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, name);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiAtmosphereV(name, count, tokens, values);
}


extern "C"
RtVoid RiInterior(RtToken name, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, name);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiInteriorV(name, count, tokens, values);
}


extern "C"
RtVoid RiExterior(RtToken name, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, name);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiExteriorV(name, count, tokens, values);
}


extern "C"
RtVoid RiShaderLayer(RtToken type, RtToken name, RtToken layername, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, layername);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiShaderLayerV(type, name, layername, count, tokens, values);
}


extern "C"
RtVoid RiResource(RtToken handle, RtToken type, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, type);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiResourceV(handle, type, count, tokens, values);
}


extern "C"
RtVoid RiAttribute(RtToken name, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, name);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiAttributeV(name, count, tokens, values);
}


extern "C"
RtVoid RiPolygon(RtInt nvertices, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, nvertices);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiPolygonV(nvertices, count, tokens, values);
}


extern "C"
RtVoid RiGeneralPolygon(RtInt nloops, RtInt nverts[], ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, nverts);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiGeneralPolygonV(nloops, nverts, count, tokens, values);
}


extern "C"
RtVoid RiPointsPolygons(RtInt npolys, RtInt nverts[], RtInt verts[], ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, verts);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiPointsPolygonsV(npolys, nverts, verts, count, tokens, values);
}


extern "C"
RtVoid RiPointsGeneralPolygons(RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, verts);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiPointsGeneralPolygonsV(npolys, nloops, nverts, verts, count, tokens, values);
}


extern "C"
RtVoid RiPatch(RtToken type, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, type);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiPatchV(type, count, tokens, values);
}


extern "C"
RtVoid RiPatchMesh(RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, vwrap);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiPatchMeshV(type, nu, uwrap, nv, vwrap, count, tokens, values);
}


extern "C"
RtVoid RiNuPatch(RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, vmax);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiNuPatchV(nu, uorder, uknot, umin, umax, nv, vorder, vknot, vmin, vmax, count, tokens, values);
}


extern "C"
RtVoid RiSubdivisionMesh(RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, floatargs);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiSubdivisionMeshV(scheme, nfaces, nvertices, vertices, ntags, tags, nargs, intargs, floatargs, count, tokens, values);
}


extern "C"
RtVoid RiSphere(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, thetamax);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiSphereV(radius, zmin, zmax, thetamax, count, tokens, values);
}


extern "C"
RtVoid RiCone(RtFloat height, RtFloat radius, RtFloat thetamax, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, thetamax);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiConeV(height, radius, thetamax, count, tokens, values);
}


extern "C"
RtVoid RiCylinder(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, thetamax);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiCylinderV(radius, zmin, zmax, thetamax, count, tokens, values);
}


extern "C"
RtVoid RiHyperboloid(RtPoint point1, RtPoint point2, RtFloat thetamax, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, thetamax);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiHyperboloidV(point1, point2, thetamax, count, tokens, values);
}


extern "C"
RtVoid RiParaboloid(RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, thetamax);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiParaboloidV(rmax, zmin, zmax, thetamax, count, tokens, values);
}


extern "C"
RtVoid RiDisk(RtFloat height, RtFloat radius, RtFloat thetamax, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, thetamax);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiDiskV(height, radius, thetamax, count, tokens, values);
}


extern "C"
RtVoid RiTorus(RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, thetamax);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiTorusV(majorrad, minorrad, phimin, phimax, thetamax, count, tokens, values);
}


extern "C"
RtVoid RiPoints(RtInt npoints, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, npoints);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiPointsV(npoints, count, tokens, values);
}


extern "C"
RtVoid RiCurves(RtToken type, RtInt ncurves, RtInt nvertices[], RtToken wrap, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, wrap);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiCurvesV(type, ncurves, nvertices, wrap, count, tokens, values);
}


extern "C"
RtVoid RiBlobby(RtInt nleaf, RtInt ncode, RtInt code[], RtInt nfloats, RtFloat floats[], RtInt nstrings, RtToken strings[], ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, strings);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiBlobbyV(nleaf, ncode, code, nfloats, floats, nstrings, strings, count, tokens, values);
}


extern "C"
RtVoid RiGeometry(RtToken type, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, type);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiGeometryV(type, count, tokens, values);
}


extern "C"
RtVoid RiMakeTexture(RtString imagefile, RtString texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, twidth);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiMakeTextureV(imagefile, texturefile, swrap, twrap, filterfunc, swidth, twidth, count, tokens, values);
}


extern "C"
RtVoid RiMakeLatLongEnvironment(RtString imagefile, RtString reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, twidth);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiMakeLatLongEnvironmentV(imagefile, reflfile, filterfunc, swidth, twidth, count, tokens, values);
}


extern "C"
RtVoid RiMakeCubeFaceEnvironment(RtString px, RtString nx, RtString py, RtString ny, RtString pz, RtString nz, RtString reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, twidth);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiMakeCubeFaceEnvironmentV(px, nx, py, ny, pz, nz, reflfile, fov, filterfunc, swidth, twidth, count, tokens, values);
}


extern "C"
RtVoid RiMakeShadow(RtString picfile, RtString shadowfile, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, shadowfile);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiMakeShadowV(picfile, shadowfile, count, tokens, values);
}


extern "C"
RtVoid RiMakeOcclusion(RtInt npics, RtString picfiles[], RtString shadowfile, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, shadowfile);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiMakeOcclusionV(npics, picfiles, shadowfile, count, tokens, values);
}


extern "C"
RtVoid RiReadArchive(RtToken name, RtArchiveCallback callback, ...)
{
    RtInt count; RtToken* tokens; RtPointer* values;
    va_list args;
    va_start(args, callback);
    buildParamList(args, count, tokens, values);
    va_end(args);

    return RiReadArchiveV(name, callback, count, tokens, values);
}

///[[[end]]]
// End varargs generated code.

/* vi: set et: */
