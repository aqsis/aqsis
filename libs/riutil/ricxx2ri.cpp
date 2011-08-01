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
/// \brief Converter from the Ri::Renderer interface to plain RI calls
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///

#include <aqsis/ri/ri.h>
#include <aqsis/ri/rif.h>
#include <aqsis/riutil/primvartoken.h>
#include <aqsis/util/logging.h>
#include <aqsis/util/exception.h>

#include <string.h> // for strcmp

#include "errorhandlerimpl.h"
#include "multistringbuffer.h"
#include <aqsis/riutil/ribparser.h>
#include <aqsis/riutil/ricxx.h>
#include <aqsis/riutil/ricxx2ri.h>
#include <aqsis/riutil/ricxx_filter.h>
#include <aqsis/riutil/risyms.h>

namespace Aqsis {

namespace {

FIXME
// Yes, this file won't compile at the moment.  It's somewhat bit rotted since
// it's no longer used actively for anything...
//
// Perhaps it would be better just to remove it.


// Convert param lists from ricxx format into count,tokens,values for the RI
class ParamListConverter
{
    private:
        MultiStringBuffer m_tokens;
        std::vector<RtPointer> m_values;
    public:

        // Perform the conversion
        void convertParamList(const Ri::ParamList& pList)
        {
            m_tokens.clear();
            m_values.clear();
            for(int i = 0, nparams = pList.size(); i < nparams; ++i)
            {
                const Ri::Param& param = pList[i];
                const char* name = param.name();
                RifTokenType type;
                RifTokenDetail detail;
                int arraySize;
                if(!RifGetDeclaration(const_cast<RtToken>(name),
                                      &type, &detail, &arraySize) &&
                    param.spec().type == static_cast<Ri::TypeSpec::Type>(type) &&
                    param.spec().iclass == static_cast<Ri::TypeSpec::IClass>(detail) &&
                    param.spec().arraySize == arraySize)
                {
                    // An identical token has already been declared; can use
                    // the plain name.
                    m_tokens.push_back(name);
                }
                else
                {
                    // Ugh, format the token back into an inline declaration
                    std::ostringstream fmt;
                    fmt << CqPrimvarToken(param.spec(), param.name());
                    m_tokens.push_back(fmt.str());
                }
                m_values.push_back(const_cast<RtPointer>(param.data()));
            }
        }

        // Get parameter count
        RtInt count() { return m_values.size(); }
        // Get parameter token list
        RtToken* tokens()
        {
            const std::vector<const char*>& toks = m_tokens.toCstringVec();
            return const_cast<RtToken*>(toks.empty() ? 0 : &toks[0]);
        }
        // Get parameter value list
        RtPointer* values() { return m_values.empty() ? 0 : &m_values[0]; }
};

}; // anon. namespace


//------------------------------------------------------------------------------
/// Class which translates calls to the RendermanInterface interface class into
/// calls to the traditional C API
///
/// This is mostly code generation
class RiCxxToRi : public Ri::Renderer
{
    private:
        ParamListConverter m_pListConv;

    public:
        // Public for simplicity, so the services object can set it.
        ArchiveRecordCallback archiveRecordCallback;

        // Code generator for autogenerated method declarations
        /*[[[cog
        from codegenutils import *

        riXml = parseXml(riXmlPath)

        for p in riXml.findall('Procedures/Procedure'):
            if p.findall('Rib'):
                decl = 'virtual %s;' % (riCxxMethodDecl(p),)
                cog.outl(wrapDecl(decl, 72, wrapIndent=20))
        ]]]*/
        virtual RtVoid Declare(RtConstString name, RtConstString declaration);
        virtual RtVoid FrameBegin(RtInt number);
        virtual RtVoid FrameEnd();
        virtual RtVoid WorldBegin();
        virtual RtVoid WorldEnd();
        virtual RtVoid IfBegin(RtConstString condition);
        virtual RtVoid ElseIf(RtConstString condition);
        virtual RtVoid Else();
        virtual RtVoid IfEnd();
        virtual RtVoid Format(RtInt xresolution, RtInt yresolution,
                            RtFloat pixelaspectratio);
        virtual RtVoid FrameAspectRatio(RtFloat frameratio);
        virtual RtVoid ScreenWindow(RtFloat left, RtFloat right, RtFloat bottom,
                            RtFloat top);
        virtual RtVoid CropWindow(RtFloat xmin, RtFloat xmax, RtFloat ymin,
                            RtFloat ymax);
        virtual RtVoid Projection(RtConstToken name, const ParamList& pList);
        virtual RtVoid Clipping(RtFloat cnear, RtFloat cfar);
        virtual RtVoid ClippingPlane(RtFloat x, RtFloat y, RtFloat z,
                            RtFloat nx, RtFloat ny, RtFloat nz);
        virtual RtVoid DepthOfField(RtFloat fstop, RtFloat focallength,
                            RtFloat focaldistance);
        virtual RtVoid Shutter(RtFloat opentime, RtFloat closetime);
        virtual RtVoid PixelVariance(RtFloat variance);
        virtual RtVoid PixelSamples(RtFloat xsamples, RtFloat ysamples);
        virtual RtVoid PixelFilter(RtFilterFunc function, RtFloat xwidth,
                            RtFloat ywidth);
        virtual RtVoid Exposure(RtFloat gain, RtFloat gamma);
        virtual RtVoid Imager(RtConstToken name, const ParamList& pList);
        virtual RtVoid Quantize(RtConstToken type, RtInt one, RtInt min,
                            RtInt max, RtFloat ditheramplitude);
        virtual RtVoid Display(RtConstToken name, RtConstToken type,
                            RtConstToken mode, const ParamList& pList);
        virtual RtVoid Hider(RtConstToken name, const ParamList& pList);
        virtual RtVoid ColorSamples(const FloatArray& nRGB,
                            const FloatArray& RGBn);
        virtual RtVoid RelativeDetail(RtFloat relativedetail);
        virtual RtVoid Option(RtConstToken name, const ParamList& pList);
        virtual RtVoid AttributeBegin();
        virtual RtVoid AttributeEnd();
        virtual RtVoid Color(RtConstColor Cq);
        virtual RtVoid Opacity(RtConstColor Os);
        virtual RtVoid TextureCoordinates(RtFloat s1, RtFloat t1, RtFloat s2,
                            RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4,
                            RtFloat t4);
        virtual RtVoid LightSource(RtConstToken shadername, RtConstToken name,
                            const ParamList& pList);
        virtual RtVoid AreaLightSource(RtConstToken shadername,
                            RtConstToken name, const ParamList& pList);
        virtual RtVoid Illuminate(RtConstToken name, RtBoolean onoff);
        virtual RtVoid Surface(RtConstToken name, const ParamList& pList);
        virtual RtVoid Displacement(RtConstToken name, const ParamList& pList);
        virtual RtVoid Atmosphere(RtConstToken name, const ParamList& pList);
        virtual RtVoid Interior(RtConstToken name, const ParamList& pList);
        virtual RtVoid Exterior(RtConstToken name, const ParamList& pList);
        virtual RtVoid ShaderLayer(RtConstToken type, RtConstToken name,
                            RtConstToken layername, const ParamList& pList);
        virtual RtVoid ConnectShaderLayers(RtConstToken type,
                            RtConstToken layer1, RtConstToken variable1,
                            RtConstToken layer2, RtConstToken variable2);
        virtual RtVoid ShadingRate(RtFloat size);
        virtual RtVoid ShadingInterpolation(RtConstToken type);
        virtual RtVoid Matte(RtBoolean onoff);
        virtual RtVoid Bound(RtConstBound bound);
        virtual RtVoid Detail(RtConstBound bound);
        virtual RtVoid DetailRange(RtFloat offlow, RtFloat onlow,
                            RtFloat onhigh, RtFloat offhigh);
        virtual RtVoid GeometricApproximation(RtConstToken type,
                            RtFloat value);
        virtual RtVoid Orientation(RtConstToken orientation);
        virtual RtVoid ReverseOrientation();
        virtual RtVoid Sides(RtInt nsides);
        virtual RtVoid Identity();
        virtual RtVoid Transform(RtConstMatrix transform);
        virtual RtVoid ConcatTransform(RtConstMatrix transform);
        virtual RtVoid Perspective(RtFloat fov);
        virtual RtVoid Translate(RtFloat dx, RtFloat dy, RtFloat dz);
        virtual RtVoid Rotate(RtFloat angle, RtFloat dx, RtFloat dy,
                            RtFloat dz);
        virtual RtVoid Scale(RtFloat sx, RtFloat sy, RtFloat sz);
        virtual RtVoid Skew(RtFloat angle, RtFloat dx1, RtFloat dy1,
                            RtFloat dz1, RtFloat dx2, RtFloat dy2,
                            RtFloat dz2);
        virtual RtVoid CoordinateSystem(RtConstToken space);
        virtual RtVoid CoordSysTransform(RtConstToken space);
        virtual RtVoid TransformBegin();
        virtual RtVoid TransformEnd();
        virtual RtVoid Resource(RtConstToken handle, RtConstToken type,
                            const ParamList& pList);
        virtual RtVoid ResourceBegin();
        virtual RtVoid ResourceEnd();
        virtual RtVoid Attribute(RtConstToken name, const ParamList& pList);
        virtual RtVoid Polygon(const ParamList& pList);
        virtual RtVoid GeneralPolygon(const IntArray& nverts,
                            const ParamList& pList);
        virtual RtVoid PointsPolygons(const IntArray& nverts,
                            const IntArray& verts, const ParamList& pList);
        virtual RtVoid PointsGeneralPolygons(const IntArray& nloops,
                            const IntArray& nverts, const IntArray& verts,
                            const ParamList& pList);
        virtual RtVoid Basis(RtConstBasis ubasis, RtInt ustep,
                            RtConstBasis vbasis, RtInt vstep);
        virtual RtVoid Patch(RtConstToken type, const ParamList& pList);
        virtual RtVoid PatchMesh(RtConstToken type, RtInt nu,
                            RtConstToken uwrap, RtInt nv, RtConstToken vwrap,
                            const ParamList& pList);
        virtual RtVoid NuPatch(RtInt nu, RtInt uorder, const FloatArray& uknot,
                            RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder,
                            const FloatArray& vknot, RtFloat vmin, RtFloat vmax,
                            const ParamList& pList);
        virtual RtVoid TrimCurve(const IntArray& ncurves, const IntArray& order,
                            const FloatArray& knot, const FloatArray& min,
                            const FloatArray& max, const IntArray& n,
                            const FloatArray& u, const FloatArray& v,
                            const FloatArray& w);
        virtual RtVoid SubdivisionMesh(RtConstToken scheme,
                            const IntArray& nvertices, const IntArray& vertices,
                            const TokenArray& tags, const IntArray& nargs,
                            const IntArray& intargs,
                            const FloatArray& floatargs,
                            const ParamList& pList);
        virtual RtVoid Sphere(RtFloat radius, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList);
        virtual RtVoid Cone(RtFloat height, RtFloat radius, RtFloat thetamax,
                            const ParamList& pList);
        virtual RtVoid Cylinder(RtFloat radius, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList);
        virtual RtVoid Hyperboloid(RtConstPoint point1, RtConstPoint point2,
                            RtFloat thetamax, const ParamList& pList);
        virtual RtVoid Paraboloid(RtFloat rmax, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList);
        virtual RtVoid Disk(RtFloat height, RtFloat radius, RtFloat thetamax,
                            const ParamList& pList);
        virtual RtVoid Torus(RtFloat majorrad, RtFloat minorrad, RtFloat phimin,
                            RtFloat phimax, RtFloat thetamax,
                            const ParamList& pList);
        virtual RtVoid Points(const ParamList& pList);
        virtual RtVoid Curves(RtConstToken type, const IntArray& nvertices,
                            RtConstToken wrap, const ParamList& pList);
        virtual RtVoid Blobby(RtInt nleaf, const IntArray& code,
                            const FloatArray& floats, const TokenArray& strings,
                            const ParamList& pList);
        virtual RtVoid Procedural(RtPointer data, RtConstBound bound,
                            RtProcSubdivFunc refineproc,
                            RtProcFreeFunc freeproc);
        virtual RtVoid Geometry(RtConstToken type, const ParamList& pList);
        virtual RtVoid SolidBegin(RtConstToken type);
        virtual RtVoid SolidEnd();
        virtual RtVoid ObjectBegin(RtConstToken name);
        virtual RtVoid ObjectEnd();
        virtual RtVoid ObjectInstance(RtConstToken name);
        virtual RtVoid MotionBegin(const FloatArray& times);
        virtual RtVoid MotionEnd();
        virtual RtVoid MakeTexture(RtConstString imagefile,
                            RtConstString texturefile, RtConstToken swrap,
                            RtConstToken twrap, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList);
        virtual RtVoid MakeLatLongEnvironment(RtConstString imagefile,
                            RtConstString reflfile, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList);
        virtual RtVoid MakeCubeFaceEnvironment(RtConstString px,
                            RtConstString nx, RtConstString py,
                            RtConstString ny, RtConstString pz,
                            RtConstString nz, RtConstString reflfile,
                            RtFloat fov, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList);
        virtual RtVoid MakeShadow(RtConstString picfile,
                            RtConstString shadowfile, const ParamList& pList);
        virtual RtVoid MakeOcclusion(const StringArray& picfiles,
                            RtConstString shadowfile, const ParamList& pList);
        virtual RtVoid ErrorHandler(RtErrorFunc handler);
        virtual RtVoid ReadArchive(RtConstToken name,
                            RtArchiveCallback callback,
                            const ParamList& pList);
        virtual RtVoid ArchiveBegin(RtConstToken name, const ParamList& pList);
        virtual RtVoid ArchiveEnd();
        //[[[end]]]

        virtual RtVoid ArchiveRecord(RtConstToken type, const char* string);
};

//------------------------------------------------------------------------------
// Helpers for conversion functions.

namespace {

// Count the length of the "P" array in the parameter list
int countP(const Ri::ParamList& pList)
{
    for(int i = 0, nparams = pList.size(); i < nparams; ++i)
    {
        if(!strcmp(pList[i].name(), "P"))
            return pList[i].size()/3;
    }
    AQSIS_THROW_XQERROR(XqParseError, EqE_MissingData,
            "\"P\" not found in parameter list");
    return -1;
}

// The various versions of toRiType() convert the ricxx types into RI ones.
// Mostly this consists of stripping off constness.
template<typename T> T& toRiType(T& val) { return val; }

// For RtString, RtToken
RtToken toRiType(RtConstToken val) { return const_cast<RtToken>(val); }
// For RtBound, RtColor, RtPoint,
float* toRiType(const float* val) { return const_cast<float*>(val); }
// For RtBasis, RtMatrix
typedef float F4array[4];
F4array* toRiType(const float val[][4]) { return const_cast<F4array*>(val); }

template<typename T> T* toRiType(const Ri::Array<T>& a)
{
    return const_cast<T*>(a.begin());
}
RtToken* toRiType(const Ri::StringArray& a)
{
    return const_cast<RtToken*>(a.begin());
}


// Size of an Ri::Array
template<typename T>
int size(const Ri::Array<T>& a)
{
    return a.size();
}

} // anon. namespace

//------------------------------------------------------------------------------
// RiCxxToRi Methods with custom implementations

RtVoid RiCxxToRi::MotionBegin(const FloatArray& times)
{
    ::RiMotionBeginV(size(times), toRiType(times));
}

RtVoid RiCxxToRi::ArchiveRecord(RtConstToken type, const char* string)
{
    if(archiveRecordCallback)
        archiveRecordCallback(type, string);
}

//------------------------------------------------------------------------------
// Autogenerated method definitions

/*[[[cog
from codegenutils import *
from Cheetah.Template import Template

customImpl = set((
    'MotionBegin',
))

procs = filter(lambda p: p.findall('Rib') and p.findtext('Name') not in customImpl,
               riXml.findall('Procedures/Procedure'))

methodTemplate = '''
#for $proc in $procs
#set $args = $cArgs($proc)
#set $deducedArgs = [a for a in $args if a.findall('RibValue')]
#set $procName = 'Ri' + $proc.findtext('Name')
$wrapDecl($riCxxMethodDecl($proc, className='RiCxxToRi'), 80)
{
    #if $proc.findall('Arguments/ParamList')
    m_pListConv.convertParamList(pList);
    #end if
    #for $arg in $deducedArgs
    $arg.findtext('Type') $arg.findtext('Name') = $arg.findtext('RibValue');
    #end for
    ## Construct argument list for the C API call
    #set $argList = []
    #for $arg in $args
        #set $argList += ['toRiType(%s)' % ($arg.findtext('Name'),)]
    #end for
    #if $proc.findall('Arguments/ParamList')
        #set $argList += ['m_pListConv.count()', 'm_pListConv.tokens()',
                          'm_pListConv.values()']
        #set $procName = $procName + 'V'
    #end if
    #set retStatement = 'return ::%s(%s);' % (procName, ', '.join(argList))
    return ::${procName}(${',\\n            '.join(argList)}
    );
}

#end for
'''

cog.out(str(Template(methodTemplate, searchList=locals())));

]]]*/

RtVoid RiCxxToRi::Declare(RtConstString name, RtConstString declaration)
{
    return ::RiDeclare(toRiType(name),
            toRiType(declaration)
    );
}

RtVoid RiCxxToRi::FrameBegin(RtInt number)
{
    return ::RiFrameBegin(toRiType(number)
    );
}

RtVoid RiCxxToRi::FrameEnd()
{
    return ::RiFrameEnd(
    );
}

RtVoid RiCxxToRi::WorldBegin()
{
    return ::RiWorldBegin(
    );
}

RtVoid RiCxxToRi::WorldEnd()
{
    return ::RiWorldEnd(
    );
}

RtVoid RiCxxToRi::IfBegin(RtConstString condition)
{
    return ::RiIfBegin(toRiType(condition)
    );
}

RtVoid RiCxxToRi::ElseIf(RtConstString condition)
{
    return ::RiElseIf(toRiType(condition)
    );
}

RtVoid RiCxxToRi::Else()
{
    return ::RiElse(
    );
}

RtVoid RiCxxToRi::IfEnd()
{
    return ::RiIfEnd(
    );
}

RtVoid RiCxxToRi::Format(RtInt xresolution, RtInt yresolution,
                         RtFloat pixelaspectratio)
{
    return ::RiFormat(toRiType(xresolution),
            toRiType(yresolution),
            toRiType(pixelaspectratio)
    );
}

RtVoid RiCxxToRi::FrameAspectRatio(RtFloat frameratio)
{
    return ::RiFrameAspectRatio(toRiType(frameratio)
    );
}

RtVoid RiCxxToRi::ScreenWindow(RtFloat left, RtFloat right, RtFloat bottom,
                               RtFloat top)
{
    return ::RiScreenWindow(toRiType(left),
            toRiType(right),
            toRiType(bottom),
            toRiType(top)
    );
}

RtVoid RiCxxToRi::CropWindow(RtFloat xmin, RtFloat xmax, RtFloat ymin,
                             RtFloat ymax)
{
    return ::RiCropWindow(toRiType(xmin),
            toRiType(xmax),
            toRiType(ymin),
            toRiType(ymax)
    );
}

RtVoid RiCxxToRi::Projection(RtConstToken name, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiProjectionV(toRiType(name),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Clipping(RtFloat cnear, RtFloat cfar)
{
    return ::RiClipping(toRiType(cnear),
            toRiType(cfar)
    );
}

RtVoid RiCxxToRi::ClippingPlane(RtFloat x, RtFloat y, RtFloat z, RtFloat nx,
                                RtFloat ny, RtFloat nz)
{
    return ::RiClippingPlane(toRiType(x),
            toRiType(y),
            toRiType(z),
            toRiType(nx),
            toRiType(ny),
            toRiType(nz)
    );
}

RtVoid RiCxxToRi::DepthOfField(RtFloat fstop, RtFloat focallength,
                               RtFloat focaldistance)
{
    return ::RiDepthOfField(toRiType(fstop),
            toRiType(focallength),
            toRiType(focaldistance)
    );
}

RtVoid RiCxxToRi::Shutter(RtFloat opentime, RtFloat closetime)
{
    return ::RiShutter(toRiType(opentime),
            toRiType(closetime)
    );
}

RtVoid RiCxxToRi::PixelVariance(RtFloat variance)
{
    return ::RiPixelVariance(toRiType(variance)
    );
}

RtVoid RiCxxToRi::PixelSamples(RtFloat xsamples, RtFloat ysamples)
{
    return ::RiPixelSamples(toRiType(xsamples),
            toRiType(ysamples)
    );
}

RtVoid RiCxxToRi::PixelFilter(RtFilterFunc function, RtFloat xwidth,
                              RtFloat ywidth)
{
    return ::RiPixelFilter(toRiType(function),
            toRiType(xwidth),
            toRiType(ywidth)
    );
}

RtVoid RiCxxToRi::Exposure(RtFloat gain, RtFloat gamma)
{
    return ::RiExposure(toRiType(gain),
            toRiType(gamma)
    );
}

RtVoid RiCxxToRi::Imager(RtConstToken name, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiImagerV(toRiType(name),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Quantize(RtConstToken type, RtInt one, RtInt min, RtInt max,
                           RtFloat ditheramplitude)
{
    return ::RiQuantize(toRiType(type),
            toRiType(one),
            toRiType(min),
            toRiType(max),
            toRiType(ditheramplitude)
    );
}

RtVoid RiCxxToRi::Display(RtConstToken name, RtConstToken type,
                          RtConstToken mode, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiDisplayV(toRiType(name),
            toRiType(type),
            toRiType(mode),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Hider(RtConstToken name, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiHiderV(toRiType(name),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::ColorSamples(const FloatArray& nRGB, const FloatArray& RGBn)
{
    RtInt N = size(nRGB)/3;
    return ::RiColorSamples(toRiType(N),
            toRiType(nRGB),
            toRiType(RGBn)
    );
}

RtVoid RiCxxToRi::RelativeDetail(RtFloat relativedetail)
{
    return ::RiRelativeDetail(toRiType(relativedetail)
    );
}

RtVoid RiCxxToRi::Option(RtConstToken name, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiOptionV(toRiType(name),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::AttributeBegin()
{
    return ::RiAttributeBegin(
    );
}

RtVoid RiCxxToRi::AttributeEnd()
{
    return ::RiAttributeEnd(
    );
}

RtVoid RiCxxToRi::Color(RtConstColor Cq)
{
    return ::RiColor(toRiType(Cq)
    );
}

RtVoid RiCxxToRi::Opacity(RtConstColor Os)
{
    return ::RiOpacity(toRiType(Os)
    );
}

RtVoid RiCxxToRi::TextureCoordinates(RtFloat s1, RtFloat t1, RtFloat s2,
                                     RtFloat t2, RtFloat s3, RtFloat t3,
                                     RtFloat s4, RtFloat t4)
{
    return ::RiTextureCoordinates(toRiType(s1),
            toRiType(t1),
            toRiType(s2),
            toRiType(t2),
            toRiType(s3),
            toRiType(t3),
            toRiType(s4),
            toRiType(t4)
    );
}

RtVoid RiCxxToRi::LightSource(RtConstToken shadername, RtConstToken name,
                              const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiLightSourceV(toRiType(shadername),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::AreaLightSource(RtConstToken shadername, RtConstToken name,
                                  const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiAreaLightSourceV(toRiType(shadername),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Illuminate(RtConstToken name, RtBoolean onoff)
{
    RtLightHandle light = ;
    return ::RiIlluminate(toRiType(light),
            toRiType(onoff)
    );
}

RtVoid RiCxxToRi::Surface(RtConstToken name, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiSurfaceV(toRiType(name),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Displacement(RtConstToken name, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiDisplacementV(toRiType(name),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Atmosphere(RtConstToken name, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiAtmosphereV(toRiType(name),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Interior(RtConstToken name, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiInteriorV(toRiType(name),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Exterior(RtConstToken name, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiExteriorV(toRiType(name),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::ShaderLayer(RtConstToken type, RtConstToken name,
                              RtConstToken layername, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiShaderLayerV(toRiType(type),
            toRiType(name),
            toRiType(layername),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::ConnectShaderLayers(RtConstToken type, RtConstToken layer1,
                                      RtConstToken variable1,
                                      RtConstToken layer2,
                                      RtConstToken variable2)
{
    return ::RiConnectShaderLayers(toRiType(type),
            toRiType(layer1),
            toRiType(variable1),
            toRiType(layer2),
            toRiType(variable2)
    );
}

RtVoid RiCxxToRi::ShadingRate(RtFloat size)
{
    return ::RiShadingRate(toRiType(size)
    );
}

RtVoid RiCxxToRi::ShadingInterpolation(RtConstToken type)
{
    return ::RiShadingInterpolation(toRiType(type)
    );
}

RtVoid RiCxxToRi::Matte(RtBoolean onoff)
{
    return ::RiMatte(toRiType(onoff)
    );
}

RtVoid RiCxxToRi::Bound(RtConstBound bound)
{
    return ::RiBound(toRiType(bound)
    );
}

RtVoid RiCxxToRi::Detail(RtConstBound bound)
{
    return ::RiDetail(toRiType(bound)
    );
}

RtVoid RiCxxToRi::DetailRange(RtFloat offlow, RtFloat onlow, RtFloat onhigh,
                              RtFloat offhigh)
{
    return ::RiDetailRange(toRiType(offlow),
            toRiType(onlow),
            toRiType(onhigh),
            toRiType(offhigh)
    );
}

RtVoid RiCxxToRi::GeometricApproximation(RtConstToken type, RtFloat value)
{
    return ::RiGeometricApproximation(toRiType(type),
            toRiType(value)
    );
}

RtVoid RiCxxToRi::Orientation(RtConstToken orientation)
{
    return ::RiOrientation(toRiType(orientation)
    );
}

RtVoid RiCxxToRi::ReverseOrientation()
{
    return ::RiReverseOrientation(
    );
}

RtVoid RiCxxToRi::Sides(RtInt nsides)
{
    return ::RiSides(toRiType(nsides)
    );
}

RtVoid RiCxxToRi::Identity()
{
    return ::RiIdentity(
    );
}

RtVoid RiCxxToRi::Transform(RtConstMatrix transform)
{
    return ::RiTransform(toRiType(transform)
    );
}

RtVoid RiCxxToRi::ConcatTransform(RtConstMatrix transform)
{
    return ::RiConcatTransform(toRiType(transform)
    );
}

RtVoid RiCxxToRi::Perspective(RtFloat fov)
{
    return ::RiPerspective(toRiType(fov)
    );
}

RtVoid RiCxxToRi::Translate(RtFloat dx, RtFloat dy, RtFloat dz)
{
    return ::RiTranslate(toRiType(dx),
            toRiType(dy),
            toRiType(dz)
    );
}

RtVoid RiCxxToRi::Rotate(RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz)
{
    return ::RiRotate(toRiType(angle),
            toRiType(dx),
            toRiType(dy),
            toRiType(dz)
    );
}

RtVoid RiCxxToRi::Scale(RtFloat sx, RtFloat sy, RtFloat sz)
{
    return ::RiScale(toRiType(sx),
            toRiType(sy),
            toRiType(sz)
    );
}

RtVoid RiCxxToRi::Skew(RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1,
                       RtFloat dx2, RtFloat dy2, RtFloat dz2)
{
    return ::RiSkew(toRiType(angle),
            toRiType(dx1),
            toRiType(dy1),
            toRiType(dz1),
            toRiType(dx2),
            toRiType(dy2),
            toRiType(dz2)
    );
}

RtVoid RiCxxToRi::CoordinateSystem(RtConstToken space)
{
    return ::RiCoordinateSystem(toRiType(space)
    );
}

RtVoid RiCxxToRi::CoordSysTransform(RtConstToken space)
{
    return ::RiCoordSysTransform(toRiType(space)
    );
}

RtVoid RiCxxToRi::TransformBegin()
{
    return ::RiTransformBegin(
    );
}

RtVoid RiCxxToRi::TransformEnd()
{
    return ::RiTransformEnd(
    );
}

RtVoid RiCxxToRi::Resource(RtConstToken handle, RtConstToken type,
                           const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiResourceV(toRiType(handle),
            toRiType(type),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::ResourceBegin()
{
    return ::RiResourceBegin(
    );
}

RtVoid RiCxxToRi::ResourceEnd()
{
    return ::RiResourceEnd(
    );
}

RtVoid RiCxxToRi::Attribute(RtConstToken name, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiAttributeV(toRiType(name),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Polygon(const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    RtInt nvertices = countP(pList);
    return ::RiPolygonV(toRiType(nvertices),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::GeneralPolygon(const IntArray& nverts,
                                 const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    RtInt nloops = size(nverts);
    return ::RiGeneralPolygonV(toRiType(nloops),
            toRiType(nverts),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::PointsPolygons(const IntArray& nverts, const IntArray& verts,
                                 const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    RtInt npolys = size(nverts);
    return ::RiPointsPolygonsV(toRiType(npolys),
            toRiType(nverts),
            toRiType(verts),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::PointsGeneralPolygons(const IntArray& nloops,
                                        const IntArray& nverts,
                                        const IntArray& verts,
                                        const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    RtInt npolys = size(nloops);
    return ::RiPointsGeneralPolygonsV(toRiType(npolys),
            toRiType(nloops),
            toRiType(nverts),
            toRiType(verts),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Basis(RtConstBasis ubasis, RtInt ustep, RtConstBasis vbasis,
                        RtInt vstep)
{
    return ::RiBasis(toRiType(ubasis),
            toRiType(ustep),
            toRiType(vbasis),
            toRiType(vstep)
    );
}

RtVoid RiCxxToRi::Patch(RtConstToken type, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiPatchV(toRiType(type),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::PatchMesh(RtConstToken type, RtInt nu, RtConstToken uwrap,
                            RtInt nv, RtConstToken vwrap,
                            const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiPatchMeshV(toRiType(type),
            toRiType(nu),
            toRiType(uwrap),
            toRiType(nv),
            toRiType(vwrap),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::NuPatch(RtInt nu, RtInt uorder, const FloatArray& uknot,
                          RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder,
                          const FloatArray& vknot, RtFloat vmin, RtFloat vmax,
                          const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiNuPatchV(toRiType(nu),
            toRiType(uorder),
            toRiType(uknot),
            toRiType(umin),
            toRiType(umax),
            toRiType(nv),
            toRiType(vorder),
            toRiType(vknot),
            toRiType(vmin),
            toRiType(vmax),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::TrimCurve(const IntArray& ncurves, const IntArray& order,
                            const FloatArray& knot, const FloatArray& min,
                            const FloatArray& max, const IntArray& n,
                            const FloatArray& u, const FloatArray& v,
                            const FloatArray& w)
{
    RtInt nloops = size(ncurves);
    return ::RiTrimCurve(toRiType(nloops),
            toRiType(ncurves),
            toRiType(order),
            toRiType(knot),
            toRiType(min),
            toRiType(max),
            toRiType(n),
            toRiType(u),
            toRiType(v),
            toRiType(w)
    );
}

RtVoid RiCxxToRi::SubdivisionMesh(RtConstToken scheme,
                                  const IntArray& nvertices,
                                  const IntArray& vertices,
                                  const TokenArray& tags, const IntArray& nargs,
                                  const IntArray& intargs,
                                  const FloatArray& floatargs,
                                  const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    RtInt nfaces = size(nvertices);
    RtInt ntags = size(tags);
    return ::RiSubdivisionMeshV(toRiType(scheme),
            toRiType(nfaces),
            toRiType(nvertices),
            toRiType(vertices),
            toRiType(ntags),
            toRiType(tags),
            toRiType(nargs),
            toRiType(intargs),
            toRiType(floatargs),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Sphere(RtFloat radius, RtFloat zmin, RtFloat zmax,
                         RtFloat thetamax, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiSphereV(toRiType(radius),
            toRiType(zmin),
            toRiType(zmax),
            toRiType(thetamax),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Cone(RtFloat height, RtFloat radius, RtFloat thetamax,
                       const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiConeV(toRiType(height),
            toRiType(radius),
            toRiType(thetamax),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Cylinder(RtFloat radius, RtFloat zmin, RtFloat zmax,
                           RtFloat thetamax, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiCylinderV(toRiType(radius),
            toRiType(zmin),
            toRiType(zmax),
            toRiType(thetamax),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Hyperboloid(RtConstPoint point1, RtConstPoint point2,
                              RtFloat thetamax, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiHyperboloidV(toRiType(point1),
            toRiType(point2),
            toRiType(thetamax),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Paraboloid(RtFloat rmax, RtFloat zmin, RtFloat zmax,
                             RtFloat thetamax, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiParaboloidV(toRiType(rmax),
            toRiType(zmin),
            toRiType(zmax),
            toRiType(thetamax),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Disk(RtFloat height, RtFloat radius, RtFloat thetamax,
                       const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiDiskV(toRiType(height),
            toRiType(radius),
            toRiType(thetamax),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Torus(RtFloat majorrad, RtFloat minorrad, RtFloat phimin,
                        RtFloat phimax, RtFloat thetamax,
                        const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiTorusV(toRiType(majorrad),
            toRiType(minorrad),
            toRiType(phimin),
            toRiType(phimax),
            toRiType(thetamax),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Points(const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    RtInt npoints = countP(pList);
    return ::RiPointsV(toRiType(npoints),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Curves(RtConstToken type, const IntArray& nvertices,
                         RtConstToken wrap, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    RtInt ncurves = size(nvertices);
    return ::RiCurvesV(toRiType(type),
            toRiType(ncurves),
            toRiType(nvertices),
            toRiType(wrap),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Blobby(RtInt nleaf, const IntArray& code,
                         const FloatArray& floats, const TokenArray& strings,
                         const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    RtInt ncode = size(code);
    RtInt nfloats = size(floats);
    RtInt nstrings = size(strings);
    return ::RiBlobbyV(toRiType(nleaf),
            toRiType(ncode),
            toRiType(code),
            toRiType(nfloats),
            toRiType(floats),
            toRiType(nstrings),
            toRiType(strings),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::Procedural(RtPointer data, RtConstBound bound,
                             RtProcSubdivFunc refineproc,
                             RtProcFreeFunc freeproc)
{
    return ::RiProcedural(toRiType(data),
            toRiType(bound),
            toRiType(refineproc),
            toRiType(freeproc)
    );
}

RtVoid RiCxxToRi::Geometry(RtConstToken type, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiGeometryV(toRiType(type),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::SolidBegin(RtConstToken type)
{
    return ::RiSolidBegin(toRiType(type)
    );
}

RtVoid RiCxxToRi::SolidEnd()
{
    return ::RiSolidEnd(
    );
}

RtVoid RiCxxToRi::ObjectBegin(RtConstToken name)
{
    return ::RiObjectBegin(
    );
}

RtVoid RiCxxToRi::ObjectEnd()
{
    return ::RiObjectEnd(
    );
}

RtVoid RiCxxToRi::ObjectInstance(RtConstToken name)
{
    RtObjectHandle handle = ;
    return ::RiObjectInstance(toRiType(handle)
    );
}

RtVoid RiCxxToRi::MotionEnd()
{
    return ::RiMotionEnd(
    );
}

RtVoid RiCxxToRi::MakeTexture(RtConstString imagefile,
                              RtConstString texturefile, RtConstToken swrap,
                              RtConstToken twrap, RtFilterFunc filterfunc,
                              RtFloat swidth, RtFloat twidth,
                              const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiMakeTextureV(toRiType(imagefile),
            toRiType(texturefile),
            toRiType(swrap),
            toRiType(twrap),
            toRiType(filterfunc),
            toRiType(swidth),
            toRiType(twidth),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::MakeLatLongEnvironment(RtConstString imagefile,
                                         RtConstString reflfile,
                                         RtFilterFunc filterfunc,
                                         RtFloat swidth, RtFloat twidth,
                                         const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiMakeLatLongEnvironmentV(toRiType(imagefile),
            toRiType(reflfile),
            toRiType(filterfunc),
            toRiType(swidth),
            toRiType(twidth),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::MakeCubeFaceEnvironment(RtConstString px, RtConstString nx,
                                          RtConstString py, RtConstString ny,
                                          RtConstString pz, RtConstString nz,
                                          RtConstString reflfile, RtFloat fov,
                                          RtFilterFunc filterfunc,
                                          RtFloat swidth, RtFloat twidth,
                                          const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiMakeCubeFaceEnvironmentV(toRiType(px),
            toRiType(nx),
            toRiType(py),
            toRiType(ny),
            toRiType(pz),
            toRiType(nz),
            toRiType(reflfile),
            toRiType(fov),
            toRiType(filterfunc),
            toRiType(swidth),
            toRiType(twidth),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::MakeShadow(RtConstString picfile, RtConstString shadowfile,
                             const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiMakeShadowV(toRiType(picfile),
            toRiType(shadowfile),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::MakeOcclusion(const StringArray& picfiles,
                                RtConstString shadowfile,
                                const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    RtInt npics = size(picfiles);
    return ::RiMakeOcclusionV(toRiType(npics),
            toRiType(picfiles),
            toRiType(shadowfile),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::ErrorHandler(RtErrorFunc handler)
{
    return ::RiErrorHandler(toRiType(handler)
    );
}

RtVoid RiCxxToRi::ReadArchive(RtConstToken name, RtArchiveCallback callback,
                              const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiReadArchiveV(toRiType(name),
            toRiType(callback),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::ArchiveBegin(RtConstToken name, const ParamList& pList)
{
    m_pListConv.convertParamList(pList);
    return ::RiArchiveBeginV(toRiType(name),
            m_pListConv.count(),
            m_pListConv.tokens(),
            m_pListConv.values()
    );
}

RtVoid RiCxxToRi::ArchiveEnd()
{
    return ::RiArchiveEnd(
    );
}

//[[[end]]]


//------------------------------------------------------------------------------
// RiCxxToRiServices implementation


RiCxxToRiServices::RiCxxToRiServices()
    : m_renderer(new RiCxxToRi()),
    m_errorHandler(new AqsisLogErrorHandler()),
    m_filterChain(),
    m_parser()
{ }

RtFilterFunc RiCxxToRiServices::getFilterFunc(RtConstToken name) const
{
    return getFilterFuncByName(name);
}

RtConstBasis* RiCxxToRiServices::getBasis(RtConstToken name) const
{
    return getBasisByName(name);
}

RtErrorFunc RiCxxToRiServices::getErrorFunc(RtConstToken name) const
{
    return getErrorFuncByName(name);
}

RtProcSubdivFunc RiCxxToRiServices::getProcSubdivFunc(RtConstToken name) const
{
    return getProcSubdivFuncByName(name);
}

Ri::TypeSpec RiCxxToRiServices::getDeclaration(RtConstToken token,
                                               const char** nameBegin,
                                               const char** nameEnd) const
{
    Ri::TypeSpec spec = parseDeclaration(token, nameBegin, nameEnd);
    if(spec.type == Ri::TypeSpec::Unknown)
    {
        RifTokenType type;
        RifTokenDetail detail;
        int arraySize;
        if(RifGetDeclaration(const_cast<RtToken>(*nameBegin),
                             &type, &detail, &arraySize))
            AQSIS_THROW_XQERROR(XqValidation, EqE_BadToken,
                                "token \"" << *nameBegin << "\" not declared");
        spec.type = static_cast<Ri::TypeSpec::Type>(type);
        spec.iclass = static_cast<Ri::TypeSpec::IClass>(detail);
        spec.arraySize = arraySize;
    }
    return spec;
}

void RiCxxToRiServices::addFilter(const char* name,
                                  const Ri::ParamList& filterParams)
{
    boost::shared_ptr<Ri::Renderer> filter(
            createFilter(name, *this, firstFilter(), filterParams));
    if(filter)
        m_filterChain.push_back(filter);
}

Ri::Renderer& RiCxxToRiServices::firstFilter()
{
    if(!m_filterChain.empty())
        return *m_filterChain.back();
    return *m_renderer;
}

void RiCxxToRiServices::parseRib(std::istream& ribStream, const char* name,
                                 const ArchiveRecordCallback& callback)
{
    ArchiveRecordCallback savedCallback = m_renderer->archiveRecordCallback;
    m_renderer->archiveRecordCallback = callback;
    parseRib(ribStream, name, firstFilter());
    // We need to restore the callback after parsing, so that if we've got
    // nested parseRib calls, the correct callback is used when the outer
    // parseRib invocation continues.
    m_renderer->archiveRecordCallback = savedCallback;
}

void RiCxxToRiServices::parseRib(std::istream& ribStream, const char* name,
                                 Ri::Renderer& context)
{
    if(!m_parser)
        m_parser.reset(RibParser::create(*this));
    m_parser->parseStream(ribStream, name, context);
}


} // namespace Aqsis
// vi: set et:
