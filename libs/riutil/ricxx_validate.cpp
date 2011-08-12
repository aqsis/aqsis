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
/// \brief Ri::Renderer interface validation
/// \author Chris Foster [chris42f (at) g mail (d0t) com]

#include <aqsis/riutil/ricxx_filter.h>

#include <cfloat>
#include <cmath>
#include <sstream>
#include <stack>
#include <string.h>

#include <aqsis/riutil/errorhandler.h>
#include <aqsis/riutil/interpclasscounts.h>
#include <aqsis/riutil/ricxxutil.h>
#include <aqsis/util/exception.h>

namespace Aqsis {


//------------------------------------------------------------------------------
/// Class which validates API call arguments and ordering.
///
/// This class performs a few major validation tasks:
///
/// 1) The RI is stateful, and Begin/End blocks must be correctly nested.
///    In addition, most calls can only occur in the scope of a restricted set
///    of block types.  We validate this.
/// 2) Most API calls have array arguments with lenghts which depend on the
///    details of the call.  We need to validate these to avoid reading off
///    the ends of arrays, etc.  If the arrays come from the C interface, the
///    lengths will be undetermined; in this case we compute the desired
///    length.
/// 3) The values of function arguments often need to lie in a given range, we
///    check this too.
///
/// All valid calls are passed on to the wrapped Ri::Renderer instance.
class RiCxxValidate : public Ri::Filter
{
    private:
        // Scope names for Begin/End blocks
        enum ApiScope {
            Scope_BeginEnd   = 1<<0,
            Scope_Frame      = 1<<1,
            Scope_World      = 1<<2,
            Scope_Attribute  = 1<<3,
            Scope_Transform  = 1<<4,
            Scope_Solid      = 1<<5,
            Scope_Object     = 1<<6,
            Scope_Motion     = 1<<7,
            Scope_Resource   = 1<<8,
            Scope_Archive    = 1<<9,
            Scope_Any        = ~0,
        };
        std::stack<ApiScope> m_scopeStack;

        // Attributes we need to keep track of to determine array lengths.
        struct AttrState
        {
            int ustep;
            int vstep;
            AttrState(int ustep, int vstep) : ustep(ustep), vstep(vstep) {}
        };
        std::stack<AttrState> m_attrStack;
        /// True if the attribute state isn't actually completely known from
        /// context, for example, if we're validating a RIB fragment.
        bool m_relaxedAttributeState;

        void pushAttributes()
        {
            m_attrStack.push(m_attrStack.top());
        }
        void popAttributes()
        {
            if(m_attrStack.size() > 1)
                m_attrStack.pop();
        }

        /// Return a string corresponding to the given ApiScope
        static const char* scopeString(ApiScope s);
        /// Push scope, unless current scope is Archive
        void pushScope(ApiScope newScope);
        /// Pop scope, unless currently in Archive and oldScope isn't Archive
        void popScope(ApiScope oldScope);
        /// Check that the current scope is in the set allowedScopes
        void checkScope(ApiScope allowedScopes, const char* procName) const;

        void checkArraySize(int expectedSize, int actualSize,
                            const char* name, const char* procName);
        void checkParamListArraySizes(const Ri::ParamList& pList,
                                      const SqInterpClassCounts& iclassCounts,
                                      const char* procName);

    public:
        RiCxxValidate(bool outerScopeRelaxed)
            : m_scopeStack(),
            m_attrStack(),
            m_relaxedAttributeState(outerScopeRelaxed)
        {
            if(outerScopeRelaxed)
                m_scopeStack.push(Scope_Any);
            else
                m_scopeStack.push(Scope_BeginEnd);
            m_attrStack.push(AttrState(3,3));
        }

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

        virtual RtVoid ArchiveRecord(RtConstToken type, const char* string)
        {
            nextFilter().ArchiveRecord(type, string);
        }
};

const char* RiCxxValidate::scopeString(ApiScope s)
{
    switch(s)
    {
        case Scope_BeginEnd:   return "Outer";
        case Scope_Frame:      return "Frame";
        case Scope_World:      return "World";
        case Scope_Attribute:  return "Attribute";
        case Scope_Transform:  return "Transform";
        case Scope_Solid:      return "Solid";
        case Scope_Object:     return "Object";
        case Scope_Motion:     return "Motion";
        case Scope_Resource:   return "Resource";
        case Scope_Archive:    return "Archive";
        case Scope_Any:        return "Any";
    }
    assert(0 && "unknown scope (bug!)");
    return "unknown scope (bug!)";
}

void RiCxxValidate::pushScope(ApiScope newScope)
{
    // Disable scope pushing/popping inside an inline archive.
    if(m_scopeStack.top() == Scope_Archive && newScope != Scope_Archive)
        return;
    m_scopeStack.push(newScope);
}

void RiCxxValidate::popScope(ApiScope oldScope)
{
    if(m_scopeStack.size() == 1)
    {
        // Never pop the outer scope.  This check is necessary when validating
        // RIB fragments which possibly might have more ScopeEnd than
        // ScopeBegin calls.
        return;
    }
    ApiScope currScope = m_scopeStack.top();
    if(currScope == Scope_Archive && oldScope != Scope_Archive)
        return;
    assert(currScope == oldScope);
    m_scopeStack.pop();
}

inline void RiCxxValidate::checkScope(ApiScope allowedScopes, const char* procName) const
{
    ApiScope currScope = m_scopeStack.top();
    if(!(currScope & allowedScopes))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_IllState,
            procName << " is invalid at " << scopeString(currScope) << " scope");
    }
}

//------------------------------------------------------------------------------
// Autogenerated method implementations

#ifndef RI_EPSILON
#   define RI_EPSILON FLT_EPSILON
#endif

namespace {

void checkPointParamPresent(const Ri::ParamList& pList)
{
    for(size_t i = 0; i < pList.size(); ++i)
    {
        if(!strcmp(pList[i].name(), "P") ||
           !strcmp(pList[i].name(), "Pw"))
            return;
    }
    AQSIS_THROW_XQERROR(XqValidation, EqE_MissingData,
        "expected \"P\" or \"Pw\" in parameter list");
}

}

inline void RiCxxValidate::checkArraySize(int expectedSize, int actualSize,
                                         const char* name, const char* procName)
{
    if(actualSize < expectedSize)
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Consistency,
            "array \"" << name << "\" of length " << actualSize
            << " too short (expected length " << expectedSize << ")");
    }
    else if(actualSize > expectedSize)
    {
        services().errorHandler().warning(EqE_Consistency,
            "array \"%s\" of length %d too long (expected length %d)",
            name, actualSize, expectedSize);
    }
}

void RiCxxValidate::checkParamListArraySizes(const Ri::ParamList& pList,
                              const SqInterpClassCounts& iclassCounts,
                              const char* procName)
{
    for(size_t i = 0; i < pList.size(); ++i)
    {
        const Ri::TypeSpec& spec = pList[i].spec();
        int expectedSize = iclassCount(iclassCounts, spec.iclass) *
                             spec.storageCount();
        if(expectedSize < 0) // Required size unknown by validator; punt.
            continue;
        checkArraySize(expectedSize, pList[i].size(), pList[i].name(),
                       procName);
    }
}

/*[[[cog
from codegenutils import *
from Cheetah.Template import Template
import re


# Extra code snippets to be inserted into method implementations
def getExtraSnippet(procName):
    ignoredScopes = set(('Transform', 'If'))
    if procName == 'Basis':
        return 'AttrState& attrs = m_attrStack.top(); attrs.ustep = ustep; attrs.vstep = vstep;'
    elif procName.endswith('Begin') and procName[:-5] not in ignoredScopes:
        return 'pushAttributes();'
    elif procName.endswith('End') and procName[:-3] not in ignoredScopes:
        return 'popAttributes();'
    return None


# Determining iclass-depenedent parameter array sizes is too complicated to be
# cleanly encoded in the XML in some cases.  Here they are as custom code
# snippets, ugh:
iclassCountSnippets = {
    'PatchMesh': 'iclassCounts = patchMeshIClassCounts(type, nu, uwrap, nv, vwrap, m_attrStack.top().ustep, m_attrStack.top().vstep, !m_relaxedAttributeState);',
    'Curves': 'iclassCounts = curvesIClassCounts(type, nvertices, wrap, m_attrStack.top().vstep, !m_relaxedAttributeState);',
    'Geometry': 'iclassCounts = SqInterpClassCounts(-1,-1,-1,-1,-1);'
}

# scopes which are ignored during validation.
irrelevantScopes = set((
    'Outside',   #< only relevant to C API
    'If',        #< XML needs updating (FIXME?)
    'Resource',  #< not fully implemented yet (TODO)
))

rangeCheckOps = {
    'gt' : '>',
    'lt' : '<',
    'ge' : '>=',
    'le' : '<=',
    'ne' : '!='
}


methodTemplate = r'''
$wrapDecl($riCxxMethodDecl($proc, className='RiCxxValidate'), 80)
{
## --------------- Scope checking -------------
#if $doScopeCheck
    checkScope(ApiScope(${' | '.join($validScopeNames)}), "$procName");
#end if
## --------------- Argument checking -------------------
#for $arg in $args
  #set $argName = $arg.findtext('Name')
  ## ---------- Range check -----------
  #set $range = $arg.find('Range')
  #if $range is not None:
    #for $check in $range.getchildren()
    #set $op = $rangeCheckOps[$check.tag]
    #set $limit = $check.text
    #set $limitIsConst = $re.match('([0-9.]*|RI_EPSILON)$', $limit)
    if(!($argName $op $limit))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"$argName $op $limit\" failed "
    #if $limitIsConst
            "[$argName = " << $argName << "]"
    #else
            "[$argName = " << $argName << ", " << "$limit = " << $limit << "]";
    #end if
        );
    }
    #end for
  #end if
  ## ------------ Array length check ------------
  #if $arg.findall('Length')
    checkArraySize($arg.findtext('Length'), ${argName}.size(),
                   "$argName", "$procName");
  #end if
#end for
## ------------ Param list check -----------
#if $proc.findall('Arguments/ParamList')
 ## ----------- Param array length check --------
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
 #set $icLen = $proc.find('IClassLengths')
 #if $icLen is not None
  #if $icLen.findall('ComplicatedCustomImpl')
    $iclassCountSnippets[$procName]
  #else
   #if $icLen.findall('Uniform')
    iclassCounts.uniform = $icLen.findtext('Uniform');
   #end if
   #if $icLen.findall('Varying')
    iclassCounts.varying = $icLen.findtext('Varying');
   #end if
   #if $icLen.findall('Vertex')
    iclassCounts.vertex = $icLen.findtext('Vertex');
   #else
    iclassCounts.vertex = iclassCounts.varying;
   #end if
   #if $icLen.findall('FaceVarying')
    iclassCounts.facevarying = $icLen.findtext('FaceVarying');
   #else
    iclassCounts.facevarying = iclassCounts.varying;
   #end if
   #if $icLen.findall('FaceVertex')
    iclassCounts.facevertex = $icLen.findtext('FaceVertex');
   #else
    iclassCounts.facevertex = iclassCounts.facevarying;
   #end if
  #end if
 #end if
 #if $procName in set(('PatchMesh', 'Curves'))
 ## Avoid checking params when they depend on the attribute state and we're in
 ## an archive.
    if(m_scopeStack.top() != Scope_Archive)
 #end if
    checkParamListArraySizes(pList, iclassCounts, "$procName");
 ## ----------- Required param check --------
 #set $requiredParams = $proc.findall('Arguments/ParamList/Param')
 #if len($requiredParams) > 0
  #for $p in $requiredParams
   #if $p.text == 'P'
    checkPointParamPresent(pList);
   #else
    Fixme_cant_handle_general_params
   #end if
  #end for
 #end if
#end if
#if $extraSnippet is not None
    $extraSnippet
#end if
#if $procName.endswith('Begin') and $procName[:-5] not in $irrelevantScopes
    pushScope(Scope_$procName[:-5]);
#end if
#if $proc.findtext('ReturnType') != 'RtVoid'
    return
#end if
    nextFilter().${procName}(${', '.join($wrapperCallArgList($proc))});
#if $procName.endswith('End') and $procName[:-3] not in $irrelevantScopes
    popScope(Scope_$procName[:-3]);
#end if
}
'''


for proc in riXml.findall('Procedures/Procedure'):
    if proc.findall('Rib'):
        args = ribArgs(proc)
        procName = proc.findtext('Name')
        validScopeNames = set(['Scope_' + s.tag for s in
                               proc.findall('ValidScope/'+'*')
                               if s.tag not in irrelevantScopes])
        doScopeCheck = len(validScopeNames) > 0
        extraSnippet = getExtraSnippet(procName)
        cog.out(str(Template(methodTemplate, searchList=locals())));

]]]*/

RtVoid RiCxxValidate::Declare(RtConstString name, RtConstString declaration)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Declare");
    return
    nextFilter().Declare(name, declaration);
}

RtVoid RiCxxValidate::FrameBegin(RtInt number)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Archive), "FrameBegin");
    pushAttributes();
    pushScope(Scope_Frame);
    nextFilter().FrameBegin(number);
}

RtVoid RiCxxValidate::FrameEnd()
{
    checkScope(ApiScope(Scope_Frame | Scope_Archive), "FrameEnd");
    popAttributes();
    nextFilter().FrameEnd();
    popScope(Scope_Frame);
}

RtVoid RiCxxValidate::WorldBegin()
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "WorldBegin");
    pushAttributes();
    pushScope(Scope_World);
    nextFilter().WorldBegin();
}

RtVoid RiCxxValidate::WorldEnd()
{
    checkScope(ApiScope(Scope_Archive | Scope_World), "WorldEnd");
    popAttributes();
    nextFilter().WorldEnd();
    popScope(Scope_World);
}

RtVoid RiCxxValidate::IfBegin(RtConstString condition)
{
    nextFilter().IfBegin(condition);
}

RtVoid RiCxxValidate::ElseIf(RtConstString condition)
{
    nextFilter().ElseIf(condition);
}

RtVoid RiCxxValidate::Else()
{
    nextFilter().Else();
}

RtVoid RiCxxValidate::IfEnd()
{
    nextFilter().IfEnd();
}

RtVoid RiCxxValidate::Format(RtInt xresolution, RtInt yresolution,
                             RtFloat pixelaspectratio)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "Format");
    if(!(xresolution != 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"xresolution != 0\" failed "
            "[xresolution = " << xresolution << "]"
        );
    }
    if(!(yresolution != 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"yresolution != 0\" failed "
            "[yresolution = " << yresolution << "]"
        );
    }
    if(!(pixelaspectratio != 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"pixelaspectratio != 0\" failed "
            "[pixelaspectratio = " << pixelaspectratio << "]"
        );
    }
    nextFilter().Format(xresolution, yresolution, pixelaspectratio);
}

RtVoid RiCxxValidate::FrameAspectRatio(RtFloat frameratio)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "FrameAspectRatio");
    if(!(frameratio > 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"frameratio > 0\" failed "
            "[frameratio = " << frameratio << "]"
        );
    }
    nextFilter().FrameAspectRatio(frameratio);
}

RtVoid RiCxxValidate::ScreenWindow(RtFloat left, RtFloat right, RtFloat bottom,
                                   RtFloat top)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "ScreenWindow");
    if(!(left < right))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"left < right\" failed "
            "[left = " << left << ", " << "right = " << right << "]";
        );
    }
    if(!(bottom < top))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"bottom < top\" failed "
            "[bottom = " << bottom << ", " << "top = " << top << "]";
        );
    }
    nextFilter().ScreenWindow(left, right, bottom, top);
}

RtVoid RiCxxValidate::CropWindow(RtFloat xmin, RtFloat xmax, RtFloat ymin,
                                 RtFloat ymax)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "CropWindow");
    if(!(xmin >= 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"xmin >= 0\" failed "
            "[xmin = " << xmin << "]"
        );
    }
    if(!(xmin < xmax))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"xmin < xmax\" failed "
            "[xmin = " << xmin << ", " << "xmax = " << xmax << "]";
        );
    }
    if(!(xmax <= 1))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"xmax <= 1\" failed "
            "[xmax = " << xmax << "]"
        );
    }
    if(!(ymin >= 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"ymin >= 0\" failed "
            "[ymin = " << ymin << "]"
        );
    }
    if(!(ymin < ymax))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"ymin < ymax\" failed "
            "[ymin = " << ymin << ", " << "ymax = " << ymax << "]";
        );
    }
    if(!(ymax <= 1))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"ymax <= 1\" failed "
            "[ymax = " << ymax << "]"
        );
    }
    nextFilter().CropWindow(xmin, xmax, ymin, ymax);
}

RtVoid RiCxxValidate::Projection(RtConstToken name, const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "Projection");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "Projection");
    nextFilter().Projection(name, pList);
}

RtVoid RiCxxValidate::Clipping(RtFloat cnear, RtFloat cfar)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "Clipping");
    if(!(cnear >= RI_EPSILON))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"cnear >= RI_EPSILON\" failed "
            "[cnear = " << cnear << "]"
        );
    }
    if(!(cfar > cnear))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"cfar > cnear\" failed "
            "[cfar = " << cfar << ", " << "cnear = " << cnear << "]";
        );
    }
    nextFilter().Clipping(cnear, cfar);
}

RtVoid RiCxxValidate::ClippingPlane(RtFloat x, RtFloat y, RtFloat z, RtFloat nx,
                                    RtFloat ny, RtFloat nz)
{
    nextFilter().ClippingPlane(x, y, z, nx, ny, nz);
}

RtVoid RiCxxValidate::DepthOfField(RtFloat fstop, RtFloat focallength,
                                   RtFloat focaldistance)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "DepthOfField");
    if(!(fstop > 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"fstop > 0\" failed "
            "[fstop = " << fstop << "]"
        );
    }
    if(!(focallength > 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"focallength > 0\" failed "
            "[focallength = " << focallength << "]"
        );
    }
    if(!(focaldistance > 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"focaldistance > 0\" failed "
            "[focaldistance = " << focaldistance << "]"
        );
    }
    nextFilter().DepthOfField(fstop, focallength, focaldistance);
}

RtVoid RiCxxValidate::Shutter(RtFloat opentime, RtFloat closetime)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "Shutter");
    if(!(opentime <= closetime))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"opentime <= closetime\" failed "
            "[opentime = " << opentime << ", " << "closetime = " << closetime << "]";
        );
    }
    nextFilter().Shutter(opentime, closetime);
}

RtVoid RiCxxValidate::PixelVariance(RtFloat variance)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "PixelVariance");
    if(!(variance >= 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"variance >= 0\" failed "
            "[variance = " << variance << "]"
        );
    }
    nextFilter().PixelVariance(variance);
}

RtVoid RiCxxValidate::PixelSamples(RtFloat xsamples, RtFloat ysamples)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "PixelSamples");
    if(!(xsamples >= 1))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"xsamples >= 1\" failed "
            "[xsamples = " << xsamples << "]"
        );
    }
    if(!(ysamples >= 1))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"ysamples >= 1\" failed "
            "[ysamples = " << ysamples << "]"
        );
    }
    nextFilter().PixelSamples(xsamples, ysamples);
}

RtVoid RiCxxValidate::PixelFilter(RtFilterFunc function, RtFloat xwidth,
                                  RtFloat ywidth)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "PixelFilter");
    if(!(xwidth > 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"xwidth > 0\" failed "
            "[xwidth = " << xwidth << "]"
        );
    }
    if(!(ywidth > 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"ywidth > 0\" failed "
            "[ywidth = " << ywidth << "]"
        );
    }
    nextFilter().PixelFilter(function, xwidth, ywidth);
}

RtVoid RiCxxValidate::Exposure(RtFloat gain, RtFloat gamma)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "Exposure");
    if(!(gain > 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"gain > 0\" failed "
            "[gain = " << gain << "]"
        );
    }
    if(!(gamma > 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"gamma > 0\" failed "
            "[gamma = " << gamma << "]"
        );
    }
    nextFilter().Exposure(gain, gamma);
}

RtVoid RiCxxValidate::Imager(RtConstToken name, const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "Imager");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "Imager");
    nextFilter().Imager(name, pList);
}

RtVoid RiCxxValidate::Quantize(RtConstToken type, RtInt one, RtInt min,
                               RtInt max, RtFloat ditheramplitude)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "Quantize");
    if(!(one >= 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"one >= 0\" failed "
            "[one = " << one << "]"
        );
    }
    if(!(min <= max))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"min <= max\" failed "
            "[min = " << min << ", " << "max = " << max << "]";
        );
    }
    if(!(ditheramplitude >= 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"ditheramplitude >= 0\" failed "
            "[ditheramplitude = " << ditheramplitude << "]"
        );
    }
    nextFilter().Quantize(type, one, min, max, ditheramplitude);
}

RtVoid RiCxxValidate::Display(RtConstToken name, RtConstToken type,
                              RtConstToken mode, const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "Display");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "Display");
    nextFilter().Display(name, type, mode, pList);
}

RtVoid RiCxxValidate::Hider(RtConstToken name, const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "Hider");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "Hider");
    nextFilter().Hider(name, pList);
}

RtVoid RiCxxValidate::ColorSamples(const FloatArray& nRGB,
                                   const FloatArray& RGBn)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "ColorSamples");
    checkArraySize(size(nRGB), RGBn.size(),
                   "RGBn", "ColorSamples");
    nextFilter().ColorSamples(nRGB, RGBn);
}

RtVoid RiCxxValidate::RelativeDetail(RtFloat relativedetail)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "RelativeDetail");
    if(!(relativedetail >= 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"relativedetail >= 0\" failed "
            "[relativedetail = " << relativedetail << "]"
        );
    }
    nextFilter().RelativeDetail(relativedetail);
}

RtVoid RiCxxValidate::Option(RtConstToken name, const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "Option");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "Option");
    nextFilter().Option(name, pList);
}

RtVoid RiCxxValidate::AttributeBegin()
{
    checkScope(ApiScope(Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Archive), "AttributeBegin");
    pushAttributes();
    pushScope(Scope_Attribute);
    nextFilter().AttributeBegin();
}

RtVoid RiCxxValidate::AttributeEnd()
{
    checkScope(ApiScope(Scope_Archive | Scope_Attribute), "AttributeEnd");
    popAttributes();
    nextFilter().AttributeEnd();
    popScope(Scope_Attribute);
}

RtVoid RiCxxValidate::Color(RtConstColor Cq)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Color");
    nextFilter().Color(Cq);
}

RtVoid RiCxxValidate::Opacity(RtConstColor Os)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Opacity");
    nextFilter().Opacity(Os);
}

RtVoid RiCxxValidate::TextureCoordinates(RtFloat s1, RtFloat t1, RtFloat s2,
                                         RtFloat t2, RtFloat s3, RtFloat t3,
                                         RtFloat s4, RtFloat t4)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "TextureCoordinates");
    nextFilter().TextureCoordinates(s1, t1, s2, t2, s3, t3, s4, t4);
}

RtVoid RiCxxValidate::LightSource(RtConstToken shadername, RtConstToken name,
                                  const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "LightSource");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "LightSource");
    return
    nextFilter().LightSource(shadername, name, pList);
}

RtVoid RiCxxValidate::AreaLightSource(RtConstToken shadername,
                                      RtConstToken name,
                                      const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "AreaLightSource");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "AreaLightSource");
    return
    nextFilter().AreaLightSource(shadername, name, pList);
}

RtVoid RiCxxValidate::Illuminate(RtConstToken name, RtBoolean onoff)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Illuminate");
    nextFilter().Illuminate(name, onoff);
}

RtVoid RiCxxValidate::Surface(RtConstToken name, const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Surface");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "Surface");
    nextFilter().Surface(name, pList);
}

RtVoid RiCxxValidate::Displacement(RtConstToken name, const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Displacement");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "Displacement");
    nextFilter().Displacement(name, pList);
}

RtVoid RiCxxValidate::Atmosphere(RtConstToken name, const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Atmosphere");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "Atmosphere");
    nextFilter().Atmosphere(name, pList);
}

RtVoid RiCxxValidate::Interior(RtConstToken name, const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Interior");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "Interior");
    nextFilter().Interior(name, pList);
}

RtVoid RiCxxValidate::Exterior(RtConstToken name, const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Exterior");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "Exterior");
    nextFilter().Exterior(name, pList);
}

RtVoid RiCxxValidate::ShaderLayer(RtConstToken type, RtConstToken name,
                                  RtConstToken layername,
                                  const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "ShaderLayer");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "ShaderLayer");
    nextFilter().ShaderLayer(type, name, layername, pList);
}

RtVoid RiCxxValidate::ConnectShaderLayers(RtConstToken type,
                                          RtConstToken layer1,
                                          RtConstToken variable1,
                                          RtConstToken layer2,
                                          RtConstToken variable2)
{
    nextFilter().ConnectShaderLayers(type, layer1, variable1, layer2, variable2);
}

RtVoid RiCxxValidate::ShadingRate(RtFloat size)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "ShadingRate");
    if(!(size > 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"size > 0\" failed "
            "[size = " << size << "]"
        );
    }
    nextFilter().ShadingRate(size);
}

RtVoid RiCxxValidate::ShadingInterpolation(RtConstToken type)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "ShadingInterpolation");
    nextFilter().ShadingInterpolation(type);
}

RtVoid RiCxxValidate::Matte(RtBoolean onoff)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Matte");
    nextFilter().Matte(onoff);
}

RtVoid RiCxxValidate::Bound(RtConstBound bound)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Bound");
    nextFilter().Bound(bound);
}

RtVoid RiCxxValidate::Detail(RtConstBound bound)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Detail");
    nextFilter().Detail(bound);
}

RtVoid RiCxxValidate::DetailRange(RtFloat offlow, RtFloat onlow, RtFloat onhigh,
                                  RtFloat offhigh)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "DetailRange");
    if(!(offlow <= onlow))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"offlow <= onlow\" failed "
            "[offlow = " << offlow << ", " << "onlow = " << onlow << "]";
        );
    }
    if(!(onlow <= onhigh))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"onlow <= onhigh\" failed "
            "[onlow = " << onlow << ", " << "onhigh = " << onhigh << "]";
        );
    }
    if(!(onhigh <= offhigh))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"onhigh <= offhigh\" failed "
            "[onhigh = " << onhigh << ", " << "offhigh = " << offhigh << "]";
        );
    }
    nextFilter().DetailRange(offlow, onlow, onhigh, offhigh);
}

RtVoid RiCxxValidate::GeometricApproximation(RtConstToken type, RtFloat value)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "GeometricApproximation");
    if(!(value >= 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"value >= 0\" failed "
            "[value = " << value << "]"
        );
    }
    nextFilter().GeometricApproximation(type, value);
}

RtVoid RiCxxValidate::Orientation(RtConstToken orientation)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Orientation");
    nextFilter().Orientation(orientation);
}

RtVoid RiCxxValidate::ReverseOrientation()
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "ReverseOrientation");
    nextFilter().ReverseOrientation();
}

RtVoid RiCxxValidate::Sides(RtInt nsides)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Sides");
    nextFilter().Sides(nsides);
}

RtVoid RiCxxValidate::Identity()
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Identity");
    nextFilter().Identity();
}

RtVoid RiCxxValidate::Transform(RtConstMatrix transform)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Transform");
    nextFilter().Transform(transform);
}

RtVoid RiCxxValidate::ConcatTransform(RtConstMatrix transform)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "ConcatTransform");
    nextFilter().ConcatTransform(transform);
}

RtVoid RiCxxValidate::Perspective(RtFloat fov)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Perspective");
    if(!(fov > 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"fov > 0\" failed "
            "[fov = " << fov << "]"
        );
    }
    nextFilter().Perspective(fov);
}

RtVoid RiCxxValidate::Translate(RtFloat dx, RtFloat dy, RtFloat dz)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Translate");
    nextFilter().Translate(dx, dy, dz);
}

RtVoid RiCxxValidate::Rotate(RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Rotate");
    nextFilter().Rotate(angle, dx, dy, dz);
}

RtVoid RiCxxValidate::Scale(RtFloat sx, RtFloat sy, RtFloat sz)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Scale");
    nextFilter().Scale(sx, sy, sz);
}

RtVoid RiCxxValidate::Skew(RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1,
                           RtFloat dx2, RtFloat dy2, RtFloat dz2)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Skew");
    nextFilter().Skew(angle, dx1, dy1, dz1, dx2, dy2, dz2);
}

RtVoid RiCxxValidate::CoordinateSystem(RtConstToken space)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive), "CoordinateSystem");
    nextFilter().CoordinateSystem(space);
}

RtVoid RiCxxValidate::CoordSysTransform(RtConstToken space)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "CoordSysTransform");
    nextFilter().CoordSysTransform(space);
}

RtVoid RiCxxValidate::TransformBegin()
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive), "TransformBegin");
    pushScope(Scope_Transform);
    nextFilter().TransformBegin();
}

RtVoid RiCxxValidate::TransformEnd()
{
    checkScope(ApiScope(Scope_Transform | Scope_Archive), "TransformEnd");
    nextFilter().TransformEnd();
    popScope(Scope_Transform);
}

RtVoid RiCxxValidate::Resource(RtConstToken handle, RtConstToken type,
                               const ParamList& pList)
{
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "Resource");
    nextFilter().Resource(handle, type, pList);
}

RtVoid RiCxxValidate::ResourceBegin()
{
    pushAttributes();
    nextFilter().ResourceBegin();
}

RtVoid RiCxxValidate::ResourceEnd()
{
    popAttributes();
    nextFilter().ResourceEnd();
}

RtVoid RiCxxValidate::Attribute(RtConstToken name, const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Attribute");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "Attribute");
    nextFilter().Attribute(name, pList);
}

RtVoid RiCxxValidate::Polygon(const ParamList& pList)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "Polygon");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = countP(pList);
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    checkParamListArraySizes(pList, iclassCounts, "Polygon");
    checkPointParamPresent(pList);
    nextFilter().Polygon(pList);
}

RtVoid RiCxxValidate::GeneralPolygon(const IntArray& nverts,
                                     const ParamList& pList)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "GeneralPolygon");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = sum(nverts);
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    checkParamListArraySizes(pList, iclassCounts, "GeneralPolygon");
    checkPointParamPresent(pList);
    nextFilter().GeneralPolygon(nverts, pList);
}

RtVoid RiCxxValidate::PointsPolygons(const IntArray& nverts,
                                     const IntArray& verts,
                                     const ParamList& pList)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "PointsPolygons");
    checkArraySize(sum(nverts), verts.size(),
                   "verts", "PointsPolygons");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.uniform = size(nverts);
    iclassCounts.varying = max(verts)+1;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = sum(nverts);
    iclassCounts.facevertex = iclassCounts.facevarying;
    checkParamListArraySizes(pList, iclassCounts, "PointsPolygons");
    checkPointParamPresent(pList);
    nextFilter().PointsPolygons(nverts, verts, pList);
}

RtVoid RiCxxValidate::PointsGeneralPolygons(const IntArray& nloops,
                                            const IntArray& nverts,
                                            const IntArray& verts,
                                            const ParamList& pList)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "PointsGeneralPolygons");
    checkArraySize(sum(nloops), nverts.size(),
                   "nverts", "PointsGeneralPolygons");
    checkArraySize(sum(nverts), verts.size(),
                   "verts", "PointsGeneralPolygons");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.uniform = size(nloops);
    iclassCounts.varying = max(verts)+1;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = sum(nverts);
    iclassCounts.facevertex = iclassCounts.facevarying;
    checkParamListArraySizes(pList, iclassCounts, "PointsGeneralPolygons");
    nextFilter().PointsGeneralPolygons(nloops, nverts, verts, pList);
}

RtVoid RiCxxValidate::Basis(RtConstBasis ubasis, RtInt ustep,
                            RtConstBasis vbasis, RtInt vstep)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "Basis");
    if(!(ustep > 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"ustep > 0\" failed "
            "[ustep = " << ustep << "]"
        );
    }
    if(!(vstep > 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"vstep > 0\" failed "
            "[vstep = " << vstep << "]"
        );
    }
    AttrState& attrs = m_attrStack.top(); attrs.ustep = ustep; attrs.vstep = vstep;
    nextFilter().Basis(ubasis, ustep, vbasis, vstep);
}

RtVoid RiCxxValidate::Patch(RtConstToken type, const ParamList& pList)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "Patch");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = 4;
    iclassCounts.vertex = strcmp(type,"bilinear")==0 ? 4 : 16;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    checkParamListArraySizes(pList, iclassCounts, "Patch");
    nextFilter().Patch(type, pList);
}

RtVoid RiCxxValidate::PatchMesh(RtConstToken type, RtInt nu, RtConstToken uwrap,
                                RtInt nv, RtConstToken vwrap,
                                const ParamList& pList)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "PatchMesh");
    if(!(nu > 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"nu > 0\" failed "
            "[nu = " << nu << "]"
        );
    }
    if(!(nv > 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"nv > 0\" failed "
            "[nv = " << nv << "]"
        );
    }
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts = patchMeshIClassCounts(type, nu, uwrap, nv, vwrap, m_attrStack.top().ustep, m_attrStack.top().vstep, !m_relaxedAttributeState);
    if(m_scopeStack.top() != Scope_Archive)
    checkParamListArraySizes(pList, iclassCounts, "PatchMesh");
    nextFilter().PatchMesh(type, nu, uwrap, nv, vwrap, pList);
}

RtVoid RiCxxValidate::NuPatch(RtInt nu, RtInt uorder, const FloatArray& uknot,
                              RtFloat umin, RtFloat umax, RtInt nv,
                              RtInt vorder, const FloatArray& vknot,
                              RtFloat vmin, RtFloat vmax,
                              const ParamList& pList)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "NuPatch");
    if(!(nu > 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"nu > 0\" failed "
            "[nu = " << nu << "]"
        );
    }
    if(!(uorder > 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"uorder > 0\" failed "
            "[uorder = " << uorder << "]"
        );
    }
    checkArraySize(nu+uorder, uknot.size(),
                   "uknot", "NuPatch");
    if(!(umin < umax))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"umin < umax\" failed "
            "[umin = " << umin << ", " << "umax = " << umax << "]";
        );
    }
    if(!(nv > 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"nv > 0\" failed "
            "[nv = " << nv << "]"
        );
    }
    if(!(vorder > 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"vorder > 0\" failed "
            "[vorder = " << vorder << "]"
        );
    }
    checkArraySize(nv+vorder, vknot.size(),
                   "vknot", "NuPatch");
    if(!(vmin < vmax))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"vmin < vmax\" failed "
            "[vmin = " << vmin << ", " << "vmax = " << vmax << "]";
        );
    }
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.uniform = (1+nu-uorder+1)*(1+nv-vorder+1);
    iclassCounts.varying = (1+nu-uorder+1)*(1+nv-vorder+1);
    iclassCounts.vertex = nu*nv;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    checkParamListArraySizes(pList, iclassCounts, "NuPatch");
    nextFilter().NuPatch(nu, uorder, uknot, umin, umax, nv, vorder, vknot, vmin, vmax, pList);
}

RtVoid RiCxxValidate::TrimCurve(const IntArray& ncurves, const IntArray& order,
                                const FloatArray& knot, const FloatArray& min,
                                const FloatArray& max, const IntArray& n,
                                const FloatArray& u, const FloatArray& v,
                                const FloatArray& w)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "TrimCurve");
    checkArraySize(sum(ncurves), order.size(),
                   "order", "TrimCurve");
    checkArraySize(sum(order)+sum(n), knot.size(),
                   "knot", "TrimCurve");
    checkArraySize(size(order), min.size(),
                   "min", "TrimCurve");
    checkArraySize(size(order), max.size(),
                   "max", "TrimCurve");
    checkArraySize(size(order), n.size(),
                   "n", "TrimCurve");
    checkArraySize(sum(n), u.size(),
                   "u", "TrimCurve");
    checkArraySize(size(u), v.size(),
                   "v", "TrimCurve");
    checkArraySize(size(u), w.size(),
                   "w", "TrimCurve");
    nextFilter().TrimCurve(ncurves, order, knot, min, max, n, u, v, w);
}

RtVoid RiCxxValidate::SubdivisionMesh(RtConstToken scheme,
                                      const IntArray& nvertices,
                                      const IntArray& vertices,
                                      const TokenArray& tags,
                                      const IntArray& nargs,
                                      const IntArray& intargs,
                                      const FloatArray& floatargs,
                                      const ParamList& pList)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "SubdivisionMesh");
    checkArraySize(sum(nvertices), vertices.size(),
                   "vertices", "SubdivisionMesh");
    checkArraySize(2*size(tags), nargs.size(),
                   "nargs", "SubdivisionMesh");
    checkArraySize(sum(nargs,0,2), intargs.size(),
                   "intargs", "SubdivisionMesh");
    checkArraySize(sum(nargs,1,2), floatargs.size(),
                   "floatargs", "SubdivisionMesh");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.uniform = size(nvertices);
    iclassCounts.varying = max(vertices)+1;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = sum(nvertices);
    iclassCounts.facevertex = iclassCounts.facevarying;
    checkParamListArraySizes(pList, iclassCounts, "SubdivisionMesh");
    nextFilter().SubdivisionMesh(scheme, nvertices, vertices, tags, nargs, intargs, floatargs, pList);
}

RtVoid RiCxxValidate::Sphere(RtFloat radius, RtFloat zmin, RtFloat zmax,
                             RtFloat thetamax, const ParamList& pList)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "Sphere");
    if(!(radius != 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"radius != 0\" failed "
            "[radius = " << radius << "]"
        );
    }
    if(!(zmin < zmax))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"zmin < zmax\" failed "
            "[zmin = " << zmin << ", " << "zmax = " << zmax << "]";
        );
    }
    if(!(thetamax != 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"thetamax != 0\" failed "
            "[thetamax = " << thetamax << "]"
        );
    }
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = 4;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    checkParamListArraySizes(pList, iclassCounts, "Sphere");
    nextFilter().Sphere(radius, zmin, zmax, thetamax, pList);
}

RtVoid RiCxxValidate::Cone(RtFloat height, RtFloat radius, RtFloat thetamax,
                           const ParamList& pList)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "Cone");
    if(!(radius != 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"radius != 0\" failed "
            "[radius = " << radius << "]"
        );
    }
    if(!(thetamax != 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"thetamax != 0\" failed "
            "[thetamax = " << thetamax << "]"
        );
    }
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = 4;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    checkParamListArraySizes(pList, iclassCounts, "Cone");
    nextFilter().Cone(height, radius, thetamax, pList);
}

RtVoid RiCxxValidate::Cylinder(RtFloat radius, RtFloat zmin, RtFloat zmax,
                               RtFloat thetamax, const ParamList& pList)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "Cylinder");
    if(!(radius != 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"radius != 0\" failed "
            "[radius = " << radius << "]"
        );
    }
    if(!(zmin != zmax))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"zmin != zmax\" failed "
            "[zmin = " << zmin << ", " << "zmax = " << zmax << "]";
        );
    }
    if(!(thetamax != 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"thetamax != 0\" failed "
            "[thetamax = " << thetamax << "]"
        );
    }
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = 4;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    checkParamListArraySizes(pList, iclassCounts, "Cylinder");
    nextFilter().Cylinder(radius, zmin, zmax, thetamax, pList);
}

RtVoid RiCxxValidate::Hyperboloid(RtConstPoint point1, RtConstPoint point2,
                                  RtFloat thetamax, const ParamList& pList)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "Hyperboloid");
    if(!(thetamax != 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"thetamax != 0\" failed "
            "[thetamax = " << thetamax << "]"
        );
    }
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = 4;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    checkParamListArraySizes(pList, iclassCounts, "Hyperboloid");
    nextFilter().Hyperboloid(point1, point2, thetamax, pList);
}

RtVoid RiCxxValidate::Paraboloid(RtFloat rmax, RtFloat zmin, RtFloat zmax,
                                 RtFloat thetamax, const ParamList& pList)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "Paraboloid");
    if(!(rmax != 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"rmax != 0\" failed "
            "[rmax = " << rmax << "]"
        );
    }
    if(!(zmin != zmax))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"zmin != zmax\" failed "
            "[zmin = " << zmin << ", " << "zmax = " << zmax << "]";
        );
    }
    if(!(thetamax != 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"thetamax != 0\" failed "
            "[thetamax = " << thetamax << "]"
        );
    }
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = 4;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    checkParamListArraySizes(pList, iclassCounts, "Paraboloid");
    nextFilter().Paraboloid(rmax, zmin, zmax, thetamax, pList);
}

RtVoid RiCxxValidate::Disk(RtFloat height, RtFloat radius, RtFloat thetamax,
                           const ParamList& pList)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "Disk");
    if(!(radius != 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"radius != 0\" failed "
            "[radius = " << radius << "]"
        );
    }
    if(!(thetamax != 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"thetamax != 0\" failed "
            "[thetamax = " << thetamax << "]"
        );
    }
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = 4;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    checkParamListArraySizes(pList, iclassCounts, "Disk");
    nextFilter().Disk(height, radius, thetamax, pList);
}

RtVoid RiCxxValidate::Torus(RtFloat majorrad, RtFloat minorrad, RtFloat phimin,
                            RtFloat phimax, RtFloat thetamax,
                            const ParamList& pList)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "Torus");
    if(!(majorrad != 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"majorrad != 0\" failed "
            "[majorrad = " << majorrad << "]"
        );
    }
    if(!(minorrad != 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"minorrad != 0\" failed "
            "[minorrad = " << minorrad << "]"
        );
    }
    if(!(phimin != phimax))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"phimin != phimax\" failed "
            "[phimin = " << phimin << ", " << "phimax = " << phimax << "]";
        );
    }
    if(!(thetamax != 0))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"thetamax != 0\" failed "
            "[thetamax = " << thetamax << "]"
        );
    }
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = 4;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    checkParamListArraySizes(pList, iclassCounts, "Torus");
    nextFilter().Torus(majorrad, minorrad, phimin, phimax, thetamax, pList);
}

RtVoid RiCxxValidate::Points(const ParamList& pList)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "Points");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = countP(pList);
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    checkParamListArraySizes(pList, iclassCounts, "Points");
    checkPointParamPresent(pList);
    nextFilter().Points(pList);
}

RtVoid RiCxxValidate::Curves(RtConstToken type, const IntArray& nvertices,
                             RtConstToken wrap, const ParamList& pList)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "Curves");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts = curvesIClassCounts(type, nvertices, wrap, m_attrStack.top().vstep, !m_relaxedAttributeState);
    if(m_scopeStack.top() != Scope_Archive)
    checkParamListArraySizes(pList, iclassCounts, "Curves");
    checkPointParamPresent(pList);
    nextFilter().Curves(type, nvertices, wrap, pList);
}

RtVoid RiCxxValidate::Blobby(RtInt nleaf, const IntArray& code,
                             const FloatArray& floats,
                             const TokenArray& strings, const ParamList& pList)
{
    checkScope(ApiScope(Scope_Motion | Scope_Object | Scope_Transform | Scope_World | Scope_Attribute | Scope_Solid | Scope_Archive), "Blobby");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts.varying = nleaf;
    iclassCounts.vertex = iclassCounts.varying;
    iclassCounts.facevarying = iclassCounts.varying;
    iclassCounts.facevertex = iclassCounts.facevarying;
    checkParamListArraySizes(pList, iclassCounts, "Blobby");
    nextFilter().Blobby(nleaf, code, floats, strings, pList);
}

RtVoid RiCxxValidate::Procedural(RtPointer data, RtConstBound bound,
                                 RtProcSubdivFunc refineproc,
                                 RtProcFreeFunc freeproc)
{
    checkScope(ApiScope(Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Archive), "Procedural");
    nextFilter().Procedural(data, bound, refineproc, freeproc);
}

RtVoid RiCxxValidate::Geometry(RtConstToken type, const ParamList& pList)
{
    checkScope(ApiScope(Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Archive), "Geometry");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts = SqInterpClassCounts(-1,-1,-1,-1,-1);
    checkParamListArraySizes(pList, iclassCounts, "Geometry");
    nextFilter().Geometry(type, pList);
}

RtVoid RiCxxValidate::SolidBegin(RtConstToken type)
{
    checkScope(ApiScope(Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Archive), "SolidBegin");
    pushAttributes();
    pushScope(Scope_Solid);
    nextFilter().SolidBegin(type);
}

RtVoid RiCxxValidate::SolidEnd()
{
    checkScope(ApiScope(Scope_Solid | Scope_Archive), "SolidEnd");
    popAttributes();
    nextFilter().SolidEnd();
    popScope(Scope_Solid);
}

RtVoid RiCxxValidate::ObjectBegin(RtConstToken name)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive), "ObjectBegin");
    pushAttributes();
    pushScope(Scope_Object);
    return
    nextFilter().ObjectBegin(name);
}

RtVoid RiCxxValidate::ObjectEnd()
{
    checkScope(ApiScope(Scope_Archive | Scope_Object), "ObjectEnd");
    popAttributes();
    nextFilter().ObjectEnd();
    popScope(Scope_Object);
}

RtVoid RiCxxValidate::ObjectInstance(RtConstToken name)
{
    checkScope(ApiScope(Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Archive), "ObjectInstance");
    nextFilter().ObjectInstance(name);
}

RtVoid RiCxxValidate::MotionBegin(const FloatArray& times)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive), "MotionBegin");
    pushAttributes();
    pushScope(Scope_Motion);
    nextFilter().MotionBegin(times);
}

RtVoid RiCxxValidate::MotionEnd()
{
    checkScope(ApiScope(Scope_Motion | Scope_Archive), "MotionEnd");
    popAttributes();
    nextFilter().MotionEnd();
    popScope(Scope_Motion);
}

RtVoid RiCxxValidate::MakeTexture(RtConstString imagefile,
                                  RtConstString texturefile, RtConstToken swrap,
                                  RtConstToken twrap, RtFilterFunc filterfunc,
                                  RtFloat swidth, RtFloat twidth,
                                  const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "MakeTexture");
    if(!(swidth >= 1))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"swidth >= 1\" failed "
            "[swidth = " << swidth << "]"
        );
    }
    if(!(twidth >= 1))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"twidth >= 1\" failed "
            "[twidth = " << twidth << "]"
        );
    }
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "MakeTexture");
    nextFilter().MakeTexture(imagefile, texturefile, swrap, twrap, filterfunc, swidth, twidth, pList);
}

RtVoid RiCxxValidate::MakeLatLongEnvironment(RtConstString imagefile,
                                             RtConstString reflfile,
                                             RtFilterFunc filterfunc,
                                             RtFloat swidth, RtFloat twidth,
                                             const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "MakeLatLongEnvironment");
    if(!(swidth >= 1))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"swidth >= 1\" failed "
            "[swidth = " << swidth << "]"
        );
    }
    if(!(twidth >= 1))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"twidth >= 1\" failed "
            "[twidth = " << twidth << "]"
        );
    }
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "MakeLatLongEnvironment");
    nextFilter().MakeLatLongEnvironment(imagefile, reflfile, filterfunc, swidth, twidth, pList);
}

RtVoid RiCxxValidate::MakeCubeFaceEnvironment(RtConstString px,
                                              RtConstString nx,
                                              RtConstString py,
                                              RtConstString ny,
                                              RtConstString pz,
                                              RtConstString nz,
                                              RtConstString reflfile,
                                              RtFloat fov,
                                              RtFilterFunc filterfunc,
                                              RtFloat swidth, RtFloat twidth,
                                              const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "MakeCubeFaceEnvironment");
    if(!(swidth >= 1))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"swidth >= 1\" failed "
            "[swidth = " << swidth << "]"
        );
    }
    if(!(twidth >= 1))
    {
        AQSIS_THROW_XQERROR(XqValidation, EqE_Range,
            "parameter check \"twidth >= 1\" failed "
            "[twidth = " << twidth << "]"
        );
    }
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "MakeCubeFaceEnvironment");
    nextFilter().MakeCubeFaceEnvironment(px, nx, py, ny, pz, nz, reflfile, fov, filterfunc, swidth, twidth, pList);
}

RtVoid RiCxxValidate::MakeShadow(RtConstString picfile,
                                 RtConstString shadowfile,
                                 const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "MakeShadow");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "MakeShadow");
    nextFilter().MakeShadow(picfile, shadowfile, pList);
}

RtVoid RiCxxValidate::MakeOcclusion(const StringArray& picfiles,
                                    RtConstString shadowfile,
                                    const ParamList& pList)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_Frame | Scope_Archive), "MakeOcclusion");
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "MakeOcclusion");
    nextFilter().MakeOcclusion(picfiles, shadowfile, pList);
}

RtVoid RiCxxValidate::ErrorHandler(RtErrorFunc handler)
{
    checkScope(ApiScope(Scope_BeginEnd | Scope_World | Scope_Object | Scope_Transform | Scope_Attribute | Scope_Solid | Scope_Frame | Scope_Archive | Scope_Motion), "ErrorHandler");
    nextFilter().ErrorHandler(handler);
}

RtVoid RiCxxValidate::ReadArchive(RtConstToken name, RtArchiveCallback callback,
                                  const ParamList& pList)
{
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "ReadArchive");
    nextFilter().ReadArchive(name, callback, pList);
}

RtVoid RiCxxValidate::ArchiveBegin(RtConstToken name, const ParamList& pList)
{
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    checkParamListArraySizes(pList, iclassCounts, "ArchiveBegin");
    pushAttributes();
    pushScope(Scope_Archive);
    return
    nextFilter().ArchiveBegin(name, pList);
}

RtVoid RiCxxValidate::ArchiveEnd()
{
    popAttributes();
    nextFilter().ArchiveEnd();
    popScope(Scope_Archive);
}
//[[[end]]]


//------------------------------------------------------------------------------
Ri::Filter* createValidateFilter(const Ri::ParamList& pList)
{
    Ri::IntArray outerScopeRelaxed = pList.findInt("relaxed_outer_scope");
    return new RiCxxValidate(outerScopeRelaxed ? outerScopeRelaxed[0] : false);
}

} // namespace Aqsis

// vi: set et:
