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
/// \brief Converter from plain RI calls to the Ri::Renderer interface.
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///

#include <aqsis/riutil/ri2ricxx.h>

#include <vector>
#include <set>
#include <stack>
#include <stdarg.h>
#include <stdio.h> // for vsnprintf
#include <boost/ptr_container/ptr_vector.hpp>

#include <aqsis/ri/ri.h>
#include <aqsis/util/exception.h>
#include <aqsis/riutil/interpclasscounts.h>
#include <aqsis/riutil/ricxx.h>
#include <aqsis/riutil/ricxxutil.h>
#include <aqsis/riutil/errorhandler.h>


using namespace Aqsis;
typedef SqInterpClassCounts IClassCounts;

namespace {

struct AttrState
{
    int ustep;
    int vstep;
    AttrState(int ustep, int vstep) : ustep(ustep), vstep(vstep) {}
};
typedef std::stack<AttrState> AttrStack;

// Class managing a symbol table of handles.
//
// We assume that FrameBegin/FrameEnd and WorldBegin/WorldEnd invalidate any
// handles on scope exit.  This class represents a stack of scopes; handles may
// be added to the set at the top of the stack, and the whole stack may be
// searched to determine whether a given handle is valid.
class HandleSetStack
{
    private:
        typedef std::set<std::string*> HSet;
        boost::ptr_vector<HSet> m_handles;

        static void deleteSetContents(HSet& set)
        {
            for(HSet::iterator i = set.begin(), iend = set.end(); i != iend; ++i)
                delete *i;
        }

    public:
        HandleSetStack()
        {
            pushScope();
        }
        ~HandleSetStack()
        {
            for(size_t i = 0; i < m_handles.size(); ++i)
                deleteSetContents(m_handles[i]);
        }

        // Push a new handle scope onto the stack
        void pushScope()
        {
            m_handles.push_back(new HSet());
        }
        // Pop a scope off the stack.
        //
        // The outer "global" scope is never popped.
        void popScope()
        {
            if(m_handles.size() > 1)
            {
                deleteSetContents(m_handles.back());
                m_handles.pop_back();
            }
        }

        // Create a new handle in the inner scope
        //
        // Return the handle name, and store the handle itself in the handle
        // parameter.
        const char* newHandle(void*& handle)
        {
            std::string* name = new std::string();
            handle = static_cast<void*>(name);
            std::ostringstream fmt;
            fmt << handle;
            (*name) = fmt.str();
            m_handles.back().insert(name);
            return name->c_str();
        }
        // Find handle name in the current stack of valid scopes.
        const char* findHandleName(void* handle) const
        {
            std::string* potentialName = static_cast<std::string*>(handle);
            for(int j = m_handles.size()-1; j >= 0; --j)
            {
                HSet::const_iterator i = m_handles[j].find(potentialName);
                if(i != m_handles[j].end())
                    return (*i)->c_str();
            }
            AQSIS_THROW_XQERROR(XqValidation, EqE_BadHandle,
                    "bad handle at " << handle);
        }
};

// Context struct for the RI->RiCxx interface conversion
struct RiToRiCxxContext
{
    AttrStack attrStack;
    HandleSetStack handles;
    Ri::RendererServices& services;

    RiToRiCxxContext(Ri::RendererServices& services)
        : attrStack(),
        handles(),
        services(services)
    {
        attrStack.push(AttrState(3,3));
    }

    void pushAttributes()
    {
        attrStack.push(attrStack.top());
    }
    void popAttributes()
    {
        if(attrStack.size() > 1)
            attrStack.pop();
    }

    Ri::Renderer& api()
    {
        // Note that we can't really cache the interface context, as filters
        // may be added to the services object at any time.
        return services.firstFilter();
    }
};

}

static RiToRiCxxContext* g_context = 0;

namespace Aqsis {

void* riToRiCxxBegin(Ri::RendererServices& services)
{
    g_context = new RiToRiCxxContext(services);
    return g_context;
}

void riToRiCxxContext(void* context)
{
    g_context = static_cast<RiToRiCxxContext*>(context);
}

void riToRiCxxEnd()
{
    delete g_context;
    g_context = 0;
}

} // namespace Aqsis

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
        Ri::TypeSpec spec = g_context->services.getDeclaration(tokens[i],
                                                    &nameBegin, &nameEnd);
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
#define EXCEPTION_CATCH_GUARD(procName)                                         \
}                                                                               \
catch(const XqValidation& e)                                                    \
{                                                                               \
    g_context->services.errorHandler().error(e.code(),                          \
           "ignoring invalid %s: %s", procName, e.what());                      \
}                                                                               \
catch(const XqException& e)                                                     \
{                                                                               \
    g_context->services.errorHandler().error(e.code(), "%s", e.what());         \
}                                                                               \
catch(const std::exception& e)                                                  \
{                                                                               \
    g_context->services.errorHandler().severe(EqE_Bug,                          \
           "std::exception encountered in %s: %s", procName, e.what());         \
}                                                                               \
catch(...)                                                                      \
{                                                                               \
    g_context->services.errorHandler().severe(EqE_Bug,                          \
           "unknown exception encountered in %s", procName);                    \
}



// -----------------------------------------------------------------------------
// C RI functions, forwarding to the C++ context object.
//
// We implement all the C interface functions as wrappers around the internal
// C++ API.  These convert parameters from the C API to the C++ one, and call
// through to the current C++ context object.  Any exceptions are caught at the
// C API boundary and passed through to the current error handler.
// -----------------------------------------------------------------------------

// Custom implementations.  These are restricted to functions which return or
// deal with handles.  The C++ API doesn't pass handles around - rather, it
// names objects with strings just like RIB.  That means we've got to do some
// extra messing around to get this right.

extern "C"
RtToken RiDeclare(RtString name, RtString declaration)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Declare(name, declaration);
    // Don't do anything special with the return value here, just return the
    // name again.
    return name;
    EXCEPTION_CATCH_GUARD("Declare")
    return 0;
}

extern "C"
RtObjectHandle RiObjectBegin()
{
    EXCEPTION_TRY_GUARD
    g_context->pushAttributes();
    void* handle = 0;
    g_context->api().ObjectBegin(g_context->handles.newHandle(handle));
    return handle;
    EXCEPTION_CATCH_GUARD("ObjectBegin")
    return 0;
}

extern "C"
RtVoid RiObjectInstance(RtObjectHandle handle)
{
    EXCEPTION_TRY_GUARD
    g_context->api().ObjectInstance(g_context->handles.findHandleName(handle));
    EXCEPTION_CATCH_GUARD("ObjectInstance")
}

extern "C"
RtLightHandle RiLightSourceV(RtToken shadername, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    void* handle = 0;
    g_context->api().LightSource(shadername, g_context->handles.newHandle(handle), pList);
    return handle;
    EXCEPTION_CATCH_GUARD("LightSource")
    return 0;
}

extern "C"
RtLightHandle RiAreaLightSourceV(RtToken shadername, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    void* handle = 0;
    g_context->api().AreaLightSource(shadername, g_context->handles.newHandle(handle), pList);
    return handle;
    EXCEPTION_CATCH_GUARD("AreaLightSource")
    return 0;
}

extern "C"
RtVoid RiIlluminate(RtLightHandle light, RtBoolean onoff)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Illuminate(g_context->handles.findHandleName(light), onoff);
    EXCEPTION_CATCH_GUARD("Illuminate")
}

extern "C"
RtArchiveHandle RiArchiveBeginV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->pushAttributes();
    g_context->api().ArchiveBegin(name, pList);
    return name;
    EXCEPTION_CATCH_GUARD("ArchiveBegin")
    return 0;
}

// RiOption{V} is commonly overridden to provide configuration outside
// RiBegin/End scope, which is why we've got the custom implementation here.
extern "C"
RtVoid RiOptionV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    if(!g_context)
    {
        riToRiCxxOptionPreBegin(name, count, tokens, values);
        return;
    }
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().Option(name, pList);
    EXCEPTION_CATCH_GUARD("Option")
}


// Autogenerated implementations.
//
// Most of the complicated stuff here is in computing appropriate array lengths
// from the C API, which uses implicit array lengths everywhere.
/*
[[[cog

from codegenutils import *
from Cheetah.Template import Template
riXml = parseXml(riXmlPath)

customImpl = set((
    'Declare',
    'ObjectBegin',
    'ObjectInstance',
    'LightSource',
    'AreaLightSource',
    'Illuminate',
    'ArchiveBegin',
    'Option',
))

pushHandleScopes = set(('FrameBegin', 'WorldBegin'))
popHandleScopes = set(('FrameEnd', 'WorldEnd'))

iclassCountSnippets = {
    'PatchMesh': 'iclassCounts = patchMeshIClassCounts(type, nu, uwrap, nv, vwrap, g_context->attrStack.top().ustep, g_context->attrStack.top().vstep);',
    'Curves': 'iclassCounts = curvesIClassCounts(type, nvertices, wrap, g_context->attrStack.top().vstep);',
    'Geometry': ''
}

# Extra code snippets to be inserted into method implementations
def getExtraSnippet(procName):
    ignoredScopes = set(('Transform', 'If'))
    if procName == 'Basis':
        return 'AttrState& attrs = g_context->attrStack.top(); attrs.ustep = ustep; attrs.vstep = vstep;'
    elif procName.endswith('Begin') and procName[:-5] not in ignoredScopes:
        return 'g_context->pushAttributes();'
    elif procName.endswith('End') and procName[:-3] not in ignoredScopes:
        return 'g_context->popAttributes();'
    return None


procTemplate = '''
extern "C"
$returnType ${cProcName}($formals)
{
    EXCEPTION_TRY_GUARD
#for $arg in $arrayArgs
    Ri::${arg.findtext('Type')[2:]} ${arg.findtext('Name')}(${arg.findtext('Name')}_in, $arrayLen(arg));
#end for
#if $proc.findall('Arguments/ParamList')
    IClassCounts iclassCounts(1,1,1,1,1);
 #set $icLen = $proc.find('IClassLengths')
 #if $icLen is not None
  #if $icLen.findall('ComplicatedCustomImpl')
    $iclassCountSnippets[$procName]
  #else
   #if $icLen.findall('Uniform')
    iclassCounts.uniform = $icLen.findtext('Uniform');
   #end if
   #if $icLen.findall('Varying')
    #if $icLen.findtext('Varying') == 'countP(pList)'
    ## special case for RiPolygon and RiPoints
    #assert $args[0].findtext('RibValue') == 'countP(pList)'
    iclassCounts.varying = $args[0].findtext('Name');
    #else
    iclassCounts.varying = $icLen.findtext('Varying');
    #end if
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
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
#end if
#if $extraSnippet is not None
    $extraSnippet
#end if
#if $procName in $pushHandleScopes
    g_context->handles.pushScope();
#end if
    g_context->api().${procName}($callArgs);
#if $procName in $popHandleScopes
    g_context->handles.popScope();
#end if
    EXCEPTION_CATCH_GUARD("$procName")
}
'''

# Get the length expression for required array arguments
def arrayLen(arg):
    if arg.findall('Length'):
        return arg.findtext('Length')
    else:
        return arg.findtext('RiLength')

for proc in riXml.findall('Procedures/Procedure'):
    procName = proc.findtext('Name')
    if proc.findall('Rib') and procName not in customImpl:
        args = cArgs(proc)
        formals = [formalArgC(arg, arraySuffix='_in') for arg in args]
        callArgsXml = [a for a in args if not a.findall('RibValue')]
        arrayArgs = [a for a in callArgsXml if
                     a.findtext('Type').endswith('Array')]
        # Ugh!  The length of 'knot' in trimcurve depends on 'n', so here's a
        # special case to swap the order.
        if procName == 'TrimCurve':
            arrayArgs[2], arrayArgs[5] = arrayArgs[5], arrayArgs[2]
        callArgs = [a.findtext('Name') for a in callArgsXml]
        returnType = proc.findtext('ReturnType')
        cProcName = cName(proc)
        if proc.findall('Arguments/ParamList'):
            cProcName = cProcName + 'V'
            callArgs.append('pList')
            formals += ['RtInt count', 'RtToken tokens[]','RtPointer values[]']
        callArgs = ', '.join(callArgs)
        formals = ', '.join(formals)
        extraSnippet = getExtraSnippet(procName)
        cog.out(str(Template(procTemplate, searchList=locals())))

]]]*/

extern "C"
RtVoid RiFrameBegin(RtInt number)
{
    EXCEPTION_TRY_GUARD
    g_context->pushAttributes();
    g_context->handles.pushScope();
    g_context->api().FrameBegin(number);
    EXCEPTION_CATCH_GUARD("FrameBegin")
}

extern "C"
RtVoid RiFrameEnd()
{
    EXCEPTION_TRY_GUARD
    g_context->popAttributes();
    g_context->api().FrameEnd();
    g_context->handles.popScope();
    EXCEPTION_CATCH_GUARD("FrameEnd")
}

extern "C"
RtVoid RiWorldBegin()
{
    EXCEPTION_TRY_GUARD
    g_context->pushAttributes();
    g_context->handles.pushScope();
    g_context->api().WorldBegin();
    EXCEPTION_CATCH_GUARD("WorldBegin")
}

extern "C"
RtVoid RiWorldEnd()
{
    EXCEPTION_TRY_GUARD
    g_context->popAttributes();
    g_context->api().WorldEnd();
    g_context->handles.popScope();
    EXCEPTION_CATCH_GUARD("WorldEnd")
}

extern "C"
RtVoid RiIfBegin(RtString condition)
{
    EXCEPTION_TRY_GUARD
    g_context->api().IfBegin(condition);
    EXCEPTION_CATCH_GUARD("IfBegin")
}

extern "C"
RtVoid RiElseIf(RtString condition)
{
    EXCEPTION_TRY_GUARD
    g_context->api().ElseIf(condition);
    EXCEPTION_CATCH_GUARD("ElseIf")
}

extern "C"
RtVoid RiElse()
{
    EXCEPTION_TRY_GUARD
    g_context->api().Else();
    EXCEPTION_CATCH_GUARD("Else")
}

extern "C"
RtVoid RiIfEnd()
{
    EXCEPTION_TRY_GUARD
    g_context->api().IfEnd();
    EXCEPTION_CATCH_GUARD("IfEnd")
}

extern "C"
RtVoid RiFormat(RtInt xresolution, RtInt yresolution, RtFloat pixelaspectratio)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Format(xresolution, yresolution, pixelaspectratio);
    EXCEPTION_CATCH_GUARD("Format")
}

extern "C"
RtVoid RiFrameAspectRatio(RtFloat frameratio)
{
    EXCEPTION_TRY_GUARD
    g_context->api().FrameAspectRatio(frameratio);
    EXCEPTION_CATCH_GUARD("FrameAspectRatio")
}

extern "C"
RtVoid RiScreenWindow(RtFloat left, RtFloat right, RtFloat bottom, RtFloat top)
{
    EXCEPTION_TRY_GUARD
    g_context->api().ScreenWindow(left, right, bottom, top);
    EXCEPTION_CATCH_GUARD("ScreenWindow")
}

extern "C"
RtVoid RiCropWindow(RtFloat xmin, RtFloat xmax, RtFloat ymin, RtFloat ymax)
{
    EXCEPTION_TRY_GUARD
    g_context->api().CropWindow(xmin, xmax, ymin, ymax);
    EXCEPTION_CATCH_GUARD("CropWindow")
}

extern "C"
RtVoid RiProjectionV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().Projection(name, pList);
    EXCEPTION_CATCH_GUARD("Projection")
}

extern "C"
RtVoid RiClipping(RtFloat cnear, RtFloat cfar)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Clipping(cnear, cfar);
    EXCEPTION_CATCH_GUARD("Clipping")
}

extern "C"
RtVoid RiClippingPlane(RtFloat x, RtFloat y, RtFloat z, RtFloat nx, RtFloat ny, RtFloat nz)
{
    EXCEPTION_TRY_GUARD
    g_context->api().ClippingPlane(x, y, z, nx, ny, nz);
    EXCEPTION_CATCH_GUARD("ClippingPlane")
}

extern "C"
RtVoid RiDepthOfField(RtFloat fstop, RtFloat focallength, RtFloat focaldistance)
{
    EXCEPTION_TRY_GUARD
    g_context->api().DepthOfField(fstop, focallength, focaldistance);
    EXCEPTION_CATCH_GUARD("DepthOfField")
}

extern "C"
RtVoid RiShutter(RtFloat opentime, RtFloat closetime)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Shutter(opentime, closetime);
    EXCEPTION_CATCH_GUARD("Shutter")
}

extern "C"
RtVoid RiPixelVariance(RtFloat variance)
{
    EXCEPTION_TRY_GUARD
    g_context->api().PixelVariance(variance);
    EXCEPTION_CATCH_GUARD("PixelVariance")
}

extern "C"
RtVoid RiPixelSamples(RtFloat xsamples, RtFloat ysamples)
{
    EXCEPTION_TRY_GUARD
    g_context->api().PixelSamples(xsamples, ysamples);
    EXCEPTION_CATCH_GUARD("PixelSamples")
}

extern "C"
RtVoid RiPixelFilter(RtFilterFunc function, RtFloat xwidth, RtFloat ywidth)
{
    EXCEPTION_TRY_GUARD
    g_context->api().PixelFilter(function, xwidth, ywidth);
    EXCEPTION_CATCH_GUARD("PixelFilter")
}

extern "C"
RtVoid RiExposure(RtFloat gain, RtFloat gamma)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Exposure(gain, gamma);
    EXCEPTION_CATCH_GUARD("Exposure")
}

extern "C"
RtVoid RiImagerV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().Imager(name, pList);
    EXCEPTION_CATCH_GUARD("Imager")
}

extern "C"
RtVoid RiQuantize(RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ditheramplitude)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Quantize(type, one, min, max, ditheramplitude);
    EXCEPTION_CATCH_GUARD("Quantize")
}

extern "C"
RtVoid RiDisplayV(RtToken name, RtToken type, RtToken mode, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().Display(name, type, mode, pList);
    EXCEPTION_CATCH_GUARD("Display")
}

extern "C"
RtVoid RiHiderV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().Hider(name, pList);
    EXCEPTION_CATCH_GUARD("Hider")
}

extern "C"
RtVoid RiColorSamples(RtInt N, RtFloat nRGB_in[], RtFloat RGBn_in[])
{
    EXCEPTION_TRY_GUARD
    Ri::FloatArray nRGB(nRGB_in, 3*N);
    Ri::FloatArray RGBn(RGBn_in, size(nRGB));
    g_context->api().ColorSamples(nRGB, RGBn);
    EXCEPTION_CATCH_GUARD("ColorSamples")
}

extern "C"
RtVoid RiRelativeDetail(RtFloat relativedetail)
{
    EXCEPTION_TRY_GUARD
    g_context->api().RelativeDetail(relativedetail);
    EXCEPTION_CATCH_GUARD("RelativeDetail")
}

extern "C"
RtVoid RiAttributeBegin()
{
    EXCEPTION_TRY_GUARD
    g_context->pushAttributes();
    g_context->api().AttributeBegin();
    EXCEPTION_CATCH_GUARD("AttributeBegin")
}

extern "C"
RtVoid RiAttributeEnd()
{
    EXCEPTION_TRY_GUARD
    g_context->popAttributes();
    g_context->api().AttributeEnd();
    EXCEPTION_CATCH_GUARD("AttributeEnd")
}

extern "C"
RtVoid RiColor(RtColor Cq)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Color(Cq);
    EXCEPTION_CATCH_GUARD("Color")
}

extern "C"
RtVoid RiOpacity(RtColor Os)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Opacity(Os);
    EXCEPTION_CATCH_GUARD("Opacity")
}

extern "C"
RtVoid RiTextureCoordinates(RtFloat s1, RtFloat t1, RtFloat s2, RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4, RtFloat t4)
{
    EXCEPTION_TRY_GUARD
    g_context->api().TextureCoordinates(s1, t1, s2, t2, s3, t3, s4, t4);
    EXCEPTION_CATCH_GUARD("TextureCoordinates")
}

extern "C"
RtVoid RiSurfaceV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().Surface(name, pList);
    EXCEPTION_CATCH_GUARD("Surface")
}

extern "C"
RtVoid RiDisplacementV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().Displacement(name, pList);
    EXCEPTION_CATCH_GUARD("Displacement")
}

extern "C"
RtVoid RiAtmosphereV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().Atmosphere(name, pList);
    EXCEPTION_CATCH_GUARD("Atmosphere")
}

extern "C"
RtVoid RiInteriorV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().Interior(name, pList);
    EXCEPTION_CATCH_GUARD("Interior")
}

extern "C"
RtVoid RiExteriorV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().Exterior(name, pList);
    EXCEPTION_CATCH_GUARD("Exterior")
}

extern "C"
RtVoid RiShaderLayerV(RtToken type, RtToken name, RtToken layername, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().ShaderLayer(type, name, layername, pList);
    EXCEPTION_CATCH_GUARD("ShaderLayer")
}

extern "C"
RtVoid RiConnectShaderLayers(RtToken type, RtToken layer1, RtToken variable1, RtToken layer2, RtToken variable2)
{
    EXCEPTION_TRY_GUARD
    g_context->api().ConnectShaderLayers(type, layer1, variable1, layer2, variable2);
    EXCEPTION_CATCH_GUARD("ConnectShaderLayers")
}

extern "C"
RtVoid RiShadingRate(RtFloat size)
{
    EXCEPTION_TRY_GUARD
    g_context->api().ShadingRate(size);
    EXCEPTION_CATCH_GUARD("ShadingRate")
}

extern "C"
RtVoid RiShadingInterpolation(RtToken type)
{
    EXCEPTION_TRY_GUARD
    g_context->api().ShadingInterpolation(type);
    EXCEPTION_CATCH_GUARD("ShadingInterpolation")
}

extern "C"
RtVoid RiMatte(RtBoolean onoff)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Matte(onoff);
    EXCEPTION_CATCH_GUARD("Matte")
}

extern "C"
RtVoid RiBound(RtBound bound)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Bound(bound);
    EXCEPTION_CATCH_GUARD("Bound")
}

extern "C"
RtVoid RiDetail(RtBound bound)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Detail(bound);
    EXCEPTION_CATCH_GUARD("Detail")
}

extern "C"
RtVoid RiDetailRange(RtFloat offlow, RtFloat onlow, RtFloat onhigh, RtFloat offhigh)
{
    EXCEPTION_TRY_GUARD
    g_context->api().DetailRange(offlow, onlow, onhigh, offhigh);
    EXCEPTION_CATCH_GUARD("DetailRange")
}

extern "C"
RtVoid RiGeometricApproximation(RtToken type, RtFloat value)
{
    EXCEPTION_TRY_GUARD
    g_context->api().GeometricApproximation(type, value);
    EXCEPTION_CATCH_GUARD("GeometricApproximation")
}

extern "C"
RtVoid RiOrientation(RtToken orientation)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Orientation(orientation);
    EXCEPTION_CATCH_GUARD("Orientation")
}

extern "C"
RtVoid RiReverseOrientation()
{
    EXCEPTION_TRY_GUARD
    g_context->api().ReverseOrientation();
    EXCEPTION_CATCH_GUARD("ReverseOrientation")
}

extern "C"
RtVoid RiSides(RtInt nsides)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Sides(nsides);
    EXCEPTION_CATCH_GUARD("Sides")
}

extern "C"
RtVoid RiIdentity()
{
    EXCEPTION_TRY_GUARD
    g_context->api().Identity();
    EXCEPTION_CATCH_GUARD("Identity")
}

extern "C"
RtVoid RiTransform(RtMatrix transform)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Transform(transform);
    EXCEPTION_CATCH_GUARD("Transform")
}

extern "C"
RtVoid RiConcatTransform(RtMatrix transform)
{
    EXCEPTION_TRY_GUARD
    g_context->api().ConcatTransform(transform);
    EXCEPTION_CATCH_GUARD("ConcatTransform")
}

extern "C"
RtVoid RiPerspective(RtFloat fov)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Perspective(fov);
    EXCEPTION_CATCH_GUARD("Perspective")
}

extern "C"
RtVoid RiTranslate(RtFloat dx, RtFloat dy, RtFloat dz)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Translate(dx, dy, dz);
    EXCEPTION_CATCH_GUARD("Translate")
}

extern "C"
RtVoid RiRotate(RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Rotate(angle, dx, dy, dz);
    EXCEPTION_CATCH_GUARD("Rotate")
}

extern "C"
RtVoid RiScale(RtFloat sx, RtFloat sy, RtFloat sz)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Scale(sx, sy, sz);
    EXCEPTION_CATCH_GUARD("Scale")
}

extern "C"
RtVoid RiSkew(RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1, RtFloat dx2, RtFloat dy2, RtFloat dz2)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Skew(angle, dx1, dy1, dz1, dx2, dy2, dz2);
    EXCEPTION_CATCH_GUARD("Skew")
}

extern "C"
RtVoid RiCoordinateSystem(RtToken space)
{
    EXCEPTION_TRY_GUARD
    g_context->api().CoordinateSystem(space);
    EXCEPTION_CATCH_GUARD("CoordinateSystem")
}

extern "C"
RtVoid RiCoordSysTransform(RtToken space)
{
    EXCEPTION_TRY_GUARD
    g_context->api().CoordSysTransform(space);
    EXCEPTION_CATCH_GUARD("CoordSysTransform")
}

extern "C"
RtVoid RiTransformBegin()
{
    EXCEPTION_TRY_GUARD
    g_context->api().TransformBegin();
    EXCEPTION_CATCH_GUARD("TransformBegin")
}

extern "C"
RtVoid RiTransformEnd()
{
    EXCEPTION_TRY_GUARD
    g_context->api().TransformEnd();
    EXCEPTION_CATCH_GUARD("TransformEnd")
}

extern "C"
RtVoid RiResourceV(RtToken handle, RtToken type, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().Resource(handle, type, pList);
    EXCEPTION_CATCH_GUARD("Resource")
}

extern "C"
RtVoid RiResourceBegin()
{
    EXCEPTION_TRY_GUARD
    g_context->pushAttributes();
    g_context->api().ResourceBegin();
    EXCEPTION_CATCH_GUARD("ResourceBegin")
}

extern "C"
RtVoid RiResourceEnd()
{
    EXCEPTION_TRY_GUARD
    g_context->popAttributes();
    g_context->api().ResourceEnd();
    EXCEPTION_CATCH_GUARD("ResourceEnd")
}

extern "C"
RtVoid RiAttributeV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().Attribute(name, pList);
    EXCEPTION_CATCH_GUARD("Attribute")
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
    g_context->api().Polygon(pList);
    EXCEPTION_CATCH_GUARD("Polygon")
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
    g_context->api().GeneralPolygon(nverts, pList);
    EXCEPTION_CATCH_GUARD("GeneralPolygon")
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
    g_context->api().PointsPolygons(nverts, verts, pList);
    EXCEPTION_CATCH_GUARD("PointsPolygons")
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
    g_context->api().PointsGeneralPolygons(nloops, nverts, verts, pList);
    EXCEPTION_CATCH_GUARD("PointsGeneralPolygons")
}

extern "C"
RtVoid RiBasis(RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep)
{
    EXCEPTION_TRY_GUARD
    AttrState& attrs = g_context->attrStack.top(); attrs.ustep = ustep; attrs.vstep = vstep;
    g_context->api().Basis(ubasis, ustep, vbasis, vstep);
    EXCEPTION_CATCH_GUARD("Basis")
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
    g_context->api().Patch(type, pList);
    EXCEPTION_CATCH_GUARD("Patch")
}

extern "C"
RtVoid RiPatchMeshV(RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts = patchMeshIClassCounts(type, nu, uwrap, nv, vwrap, g_context->attrStack.top().ustep, g_context->attrStack.top().vstep);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().PatchMesh(type, nu, uwrap, nv, vwrap, pList);
    EXCEPTION_CATCH_GUARD("PatchMesh")
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
    g_context->api().NuPatch(nu, uorder, uknot, umin, umax, nv, vorder, vknot, vmin, vmax, pList);
    EXCEPTION_CATCH_GUARD("NuPatch")
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
    g_context->api().TrimCurve(ncurves, order, knot, min, max, n, u, v, w);
    EXCEPTION_CATCH_GUARD("TrimCurve")
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
    g_context->api().SubdivisionMesh(scheme, nvertices, vertices, tags, nargs, intargs, floatargs, pList);
    EXCEPTION_CATCH_GUARD("SubdivisionMesh")
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
    g_context->api().Sphere(radius, zmin, zmax, thetamax, pList);
    EXCEPTION_CATCH_GUARD("Sphere")
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
    g_context->api().Cone(height, radius, thetamax, pList);
    EXCEPTION_CATCH_GUARD("Cone")
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
    g_context->api().Cylinder(radius, zmin, zmax, thetamax, pList);
    EXCEPTION_CATCH_GUARD("Cylinder")
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
    g_context->api().Hyperboloid(point1, point2, thetamax, pList);
    EXCEPTION_CATCH_GUARD("Hyperboloid")
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
    g_context->api().Paraboloid(rmax, zmin, zmax, thetamax, pList);
    EXCEPTION_CATCH_GUARD("Paraboloid")
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
    g_context->api().Disk(height, radius, thetamax, pList);
    EXCEPTION_CATCH_GUARD("Disk")
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
    g_context->api().Torus(majorrad, minorrad, phimin, phimax, thetamax, pList);
    EXCEPTION_CATCH_GUARD("Torus")
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
    g_context->api().Points(pList);
    EXCEPTION_CATCH_GUARD("Points")
}

extern "C"
RtVoid RiCurvesV(RtToken type, RtInt ncurves, RtInt nvertices_in[], RtToken wrap, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    Ri::IntArray nvertices(nvertices_in, ncurves);
    IClassCounts iclassCounts(1,1,1,1,1);
    iclassCounts = curvesIClassCounts(type, nvertices, wrap, g_context->attrStack.top().vstep);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().Curves(type, nvertices, wrap, pList);
    EXCEPTION_CATCH_GUARD("Curves")
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
    g_context->api().Blobby(nleaf, code, floats, strings, pList);
    EXCEPTION_CATCH_GUARD("Blobby")
}

extern "C"
RtVoid RiProcedural(RtPointer data, RtBound bound, RtProcSubdivFunc refineproc, RtProcFreeFunc freeproc)
{
    EXCEPTION_TRY_GUARD
    g_context->api().Procedural(data, bound, refineproc, freeproc);
    EXCEPTION_CATCH_GUARD("Procedural")
}

extern "C"
RtVoid RiGeometryV(RtToken type, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().Geometry(type, pList);
    EXCEPTION_CATCH_GUARD("Geometry")
}

extern "C"
RtVoid RiSolidBegin(RtToken type)
{
    EXCEPTION_TRY_GUARD
    g_context->pushAttributes();
    g_context->api().SolidBegin(type);
    EXCEPTION_CATCH_GUARD("SolidBegin")
}

extern "C"
RtVoid RiSolidEnd()
{
    EXCEPTION_TRY_GUARD
    g_context->popAttributes();
    g_context->api().SolidEnd();
    EXCEPTION_CATCH_GUARD("SolidEnd")
}

extern "C"
RtVoid RiObjectEnd()
{
    EXCEPTION_TRY_GUARD
    g_context->popAttributes();
    g_context->api().ObjectEnd();
    EXCEPTION_CATCH_GUARD("ObjectEnd")
}

extern "C"
RtVoid RiMotionBeginV(RtInt N, RtFloat times_in[])
{
    EXCEPTION_TRY_GUARD
    Ri::FloatArray times(times_in, N);
    g_context->pushAttributes();
    g_context->api().MotionBegin(times);
    EXCEPTION_CATCH_GUARD("MotionBegin")
}

extern "C"
RtVoid RiMotionEnd()
{
    EXCEPTION_TRY_GUARD
    g_context->popAttributes();
    g_context->api().MotionEnd();
    EXCEPTION_CATCH_GUARD("MotionEnd")
}

extern "C"
RtVoid RiMakeTextureV(RtString imagefile, RtString texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().MakeTexture(imagefile, texturefile, swrap, twrap, filterfunc, swidth, twidth, pList);
    EXCEPTION_CATCH_GUARD("MakeTexture")
}

extern "C"
RtVoid RiMakeLatLongEnvironmentV(RtString imagefile, RtString reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().MakeLatLongEnvironment(imagefile, reflfile, filterfunc, swidth, twidth, pList);
    EXCEPTION_CATCH_GUARD("MakeLatLongEnvironment")
}

extern "C"
RtVoid RiMakeCubeFaceEnvironmentV(RtString px, RtString nx, RtString py, RtString ny, RtString pz, RtString nz, RtString reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().MakeCubeFaceEnvironment(px, nx, py, ny, pz, nz, reflfile, fov, filterfunc, swidth, twidth, pList);
    EXCEPTION_CATCH_GUARD("MakeCubeFaceEnvironment")
}

extern "C"
RtVoid RiMakeShadowV(RtString picfile, RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().MakeShadow(picfile, shadowfile, pList);
    EXCEPTION_CATCH_GUARD("MakeShadow")
}

extern "C"
RtVoid RiMakeOcclusionV(RtInt npics, RtString picfiles_in[], RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    Ri::StringArray picfiles(picfiles_in, npics);
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().MakeOcclusion(picfiles, shadowfile, pList);
    EXCEPTION_CATCH_GUARD("MakeOcclusion")
}

extern "C"
RtVoid RiErrorHandler(RtErrorFunc handler)
{
    EXCEPTION_TRY_GUARD
    g_context->api().ErrorHandler(handler);
    EXCEPTION_CATCH_GUARD("ErrorHandler")
}

extern "C"
RtVoid RiReadArchiveV(RtToken name, RtArchiveCallback callback, RtInt count, RtToken tokens[], RtPointer values[])
{
    EXCEPTION_TRY_GUARD
    IClassCounts iclassCounts(1,1,1,1,1);
    Ri::ParamList pList = buildParamList(count, tokens, values, iclassCounts);
    g_context->api().ReadArchive(name, callback, pList);
    EXCEPTION_CATCH_GUARD("ReadArchive")
}

extern "C"
RtVoid RiArchiveEnd()
{
    EXCEPTION_TRY_GUARD
    g_context->popAttributes();
    g_context->api().ArchiveEnd();
    EXCEPTION_CATCH_GUARD("ArchiveEnd")
}
///[[[end]]]
// End main generated code
//-----------------------------------------------------------------------------

namespace {

std::vector<RtToken> g_tokens;
std::vector<RtPointer> g_values;

/// Utility to build a parameter list from a va_list.
///
/// This function makes use of global storage for the tokens and values - it's
/// _not_ threadsafe or reentrant!  That shouldn't present a problem though,
/// since the C API inherently uses globals.
///
/// args - is the the vararg list representing an RI parameter list
/// count,tokens,values - are the returned paramlist for the associated 'V'
///                       version of the RI call.
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

#define COLLECT_VARARGS(lastNamedArg)                   \
    RtInt count; RtToken* tokens; RtPointer* values;    \
    va_list args;                                       \
    va_start(args, lastNamedArg);                       \
    buildParamList(args, count, tokens, values);        \
    va_end(args)

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
    COLLECT_VARARGS(${args[-1].findtext('Name')});
    return ${procName}V($callArgs, count, tokens, values);
}
'''

for proc in riXml.findall('Procedures/Procedure'):
    if proc.findall('Rib') and proc.findall('Arguments/ParamList'):
        args = cArgs(proc)
        formals = ', '.join([formalArgC(arg) for arg in args])
        callArgs = ', '.join([arg.findtext('Name') for arg in args])
        returnType = proc.findtext('ReturnType')
        procName = cName(proc)
        cog.out(str(Template(varargsProcTemplate, searchList=locals())))


]]]*/

extern "C"
RtVoid RiProjection(RtToken name, ...)
{
    COLLECT_VARARGS(name);
    return RiProjectionV(name, count, tokens, values);
}

extern "C"
RtVoid RiImager(RtToken name, ...)
{
    COLLECT_VARARGS(name);
    return RiImagerV(name, count, tokens, values);
}

extern "C"
RtVoid RiDisplay(RtToken name, RtToken type, RtToken mode, ...)
{
    COLLECT_VARARGS(mode);
    return RiDisplayV(name, type, mode, count, tokens, values);
}

extern "C"
RtVoid RiHider(RtToken name, ...)
{
    COLLECT_VARARGS(name);
    return RiHiderV(name, count, tokens, values);
}

extern "C"
RtVoid RiOption(RtToken name, ...)
{
    COLLECT_VARARGS(name);
    return RiOptionV(name, count, tokens, values);
}

extern "C"
RtLightHandle RiLightSource(RtToken shadername, ...)
{
    COLLECT_VARARGS(shadername);
    return RiLightSourceV(shadername, count, tokens, values);
}

extern "C"
RtLightHandle RiAreaLightSource(RtToken shadername, ...)
{
    COLLECT_VARARGS(shadername);
    return RiAreaLightSourceV(shadername, count, tokens, values);
}

extern "C"
RtVoid RiSurface(RtToken name, ...)
{
    COLLECT_VARARGS(name);
    return RiSurfaceV(name, count, tokens, values);
}

extern "C"
RtVoid RiDisplacement(RtToken name, ...)
{
    COLLECT_VARARGS(name);
    return RiDisplacementV(name, count, tokens, values);
}

extern "C"
RtVoid RiAtmosphere(RtToken name, ...)
{
    COLLECT_VARARGS(name);
    return RiAtmosphereV(name, count, tokens, values);
}

extern "C"
RtVoid RiInterior(RtToken name, ...)
{
    COLLECT_VARARGS(name);
    return RiInteriorV(name, count, tokens, values);
}

extern "C"
RtVoid RiExterior(RtToken name, ...)
{
    COLLECT_VARARGS(name);
    return RiExteriorV(name, count, tokens, values);
}

extern "C"
RtVoid RiShaderLayer(RtToken type, RtToken name, RtToken layername, ...)
{
    COLLECT_VARARGS(layername);
    return RiShaderLayerV(type, name, layername, count, tokens, values);
}

extern "C"
RtVoid RiResource(RtToken handle, RtToken type, ...)
{
    COLLECT_VARARGS(type);
    return RiResourceV(handle, type, count, tokens, values);
}

extern "C"
RtVoid RiAttribute(RtToken name, ...)
{
    COLLECT_VARARGS(name);
    return RiAttributeV(name, count, tokens, values);
}

extern "C"
RtVoid RiPolygon(RtInt nvertices, ...)
{
    COLLECT_VARARGS(nvertices);
    return RiPolygonV(nvertices, count, tokens, values);
}

extern "C"
RtVoid RiGeneralPolygon(RtInt nloops, RtInt nverts[], ...)
{
    COLLECT_VARARGS(nverts);
    return RiGeneralPolygonV(nloops, nverts, count, tokens, values);
}

extern "C"
RtVoid RiPointsPolygons(RtInt npolys, RtInt nverts[], RtInt verts[], ...)
{
    COLLECT_VARARGS(verts);
    return RiPointsPolygonsV(npolys, nverts, verts, count, tokens, values);
}

extern "C"
RtVoid RiPointsGeneralPolygons(RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], ...)
{
    COLLECT_VARARGS(verts);
    return RiPointsGeneralPolygonsV(npolys, nloops, nverts, verts, count, tokens, values);
}

extern "C"
RtVoid RiPatch(RtToken type, ...)
{
    COLLECT_VARARGS(type);
    return RiPatchV(type, count, tokens, values);
}

extern "C"
RtVoid RiPatchMesh(RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, ...)
{
    COLLECT_VARARGS(vwrap);
    return RiPatchMeshV(type, nu, uwrap, nv, vwrap, count, tokens, values);
}

extern "C"
RtVoid RiNuPatch(RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, ...)
{
    COLLECT_VARARGS(vmax);
    return RiNuPatchV(nu, uorder, uknot, umin, umax, nv, vorder, vknot, vmin, vmax, count, tokens, values);
}

extern "C"
RtVoid RiSubdivisionMesh(RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], ...)
{
    COLLECT_VARARGS(floatargs);
    return RiSubdivisionMeshV(scheme, nfaces, nvertices, vertices, ntags, tags, nargs, intargs, floatargs, count, tokens, values);
}

extern "C"
RtVoid RiSphere(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, ...)
{
    COLLECT_VARARGS(thetamax);
    return RiSphereV(radius, zmin, zmax, thetamax, count, tokens, values);
}

extern "C"
RtVoid RiCone(RtFloat height, RtFloat radius, RtFloat thetamax, ...)
{
    COLLECT_VARARGS(thetamax);
    return RiConeV(height, radius, thetamax, count, tokens, values);
}

extern "C"
RtVoid RiCylinder(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, ...)
{
    COLLECT_VARARGS(thetamax);
    return RiCylinderV(radius, zmin, zmax, thetamax, count, tokens, values);
}

extern "C"
RtVoid RiHyperboloid(RtPoint point1, RtPoint point2, RtFloat thetamax, ...)
{
    COLLECT_VARARGS(thetamax);
    return RiHyperboloidV(point1, point2, thetamax, count, tokens, values);
}

extern "C"
RtVoid RiParaboloid(RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, ...)
{
    COLLECT_VARARGS(thetamax);
    return RiParaboloidV(rmax, zmin, zmax, thetamax, count, tokens, values);
}

extern "C"
RtVoid RiDisk(RtFloat height, RtFloat radius, RtFloat thetamax, ...)
{
    COLLECT_VARARGS(thetamax);
    return RiDiskV(height, radius, thetamax, count, tokens, values);
}

extern "C"
RtVoid RiTorus(RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, ...)
{
    COLLECT_VARARGS(thetamax);
    return RiTorusV(majorrad, minorrad, phimin, phimax, thetamax, count, tokens, values);
}

extern "C"
RtVoid RiPoints(RtInt npoints, ...)
{
    COLLECT_VARARGS(npoints);
    return RiPointsV(npoints, count, tokens, values);
}

extern "C"
RtVoid RiCurves(RtToken type, RtInt ncurves, RtInt nvertices[], RtToken wrap, ...)
{
    COLLECT_VARARGS(wrap);
    return RiCurvesV(type, ncurves, nvertices, wrap, count, tokens, values);
}

extern "C"
RtVoid RiBlobby(RtInt nleaf, RtInt ncode, RtInt code[], RtInt nfloats, RtFloat floats[], RtInt nstrings, RtToken strings[], ...)
{
    COLLECT_VARARGS(strings);
    return RiBlobbyV(nleaf, ncode, code, nfloats, floats, nstrings, strings, count, tokens, values);
}

extern "C"
RtVoid RiGeometry(RtToken type, ...)
{
    COLLECT_VARARGS(type);
    return RiGeometryV(type, count, tokens, values);
}

extern "C"
RtVoid RiMakeTexture(RtString imagefile, RtString texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ...)
{
    COLLECT_VARARGS(twidth);
    return RiMakeTextureV(imagefile, texturefile, swrap, twrap, filterfunc, swidth, twidth, count, tokens, values);
}

extern "C"
RtVoid RiMakeLatLongEnvironment(RtString imagefile, RtString reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ...)
{
    COLLECT_VARARGS(twidth);
    return RiMakeLatLongEnvironmentV(imagefile, reflfile, filterfunc, swidth, twidth, count, tokens, values);
}

extern "C"
RtVoid RiMakeCubeFaceEnvironment(RtString px, RtString nx, RtString py, RtString ny, RtString pz, RtString nz, RtString reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ...)
{
    COLLECT_VARARGS(twidth);
    return RiMakeCubeFaceEnvironmentV(px, nx, py, ny, pz, nz, reflfile, fov, filterfunc, swidth, twidth, count, tokens, values);
}

extern "C"
RtVoid RiMakeShadow(RtString picfile, RtString shadowfile, ...)
{
    COLLECT_VARARGS(shadowfile);
    return RiMakeShadowV(picfile, shadowfile, count, tokens, values);
}

extern "C"
RtVoid RiMakeOcclusion(RtInt npics, RtString picfiles[], RtString shadowfile, ...)
{
    COLLECT_VARARGS(shadowfile);
    return RiMakeOcclusionV(npics, picfiles, shadowfile, count, tokens, values);
}

extern "C"
RtVoid RiReadArchive(RtToken name, RtArchiveCallback callback, ...)
{
    COLLECT_VARARGS(callback);
    return RiReadArchiveV(name, callback, count, tokens, values);
}

extern "C"
RtArchiveHandle RiArchiveBegin(RtToken name, ...)
{
    COLLECT_VARARGS(name);
    return RiArchiveBeginV(name, count, tokens, values);
}
///[[[end]]]
// End varargs generated code.
//--------------------------------------------------

// This one is oh-so-odd.  Here's a custom implementation.
extern "C"
RtVoid RiMotionBegin(RtInt N, ...)
{
    va_list args;
    va_start(args, N);
    std::vector<RtFloat> times(N);
    for(int i = 0; i < N; ++i)
        times[i] = va_arg(args, double);
    va_end(args);

    return RiMotionBeginV(N, N == 0 ? 0 : &times[0]);
}

// ArchiveRecord is also an oddball.
RtVoid RiArchiveRecord(RtToken type, char *format, ...)
{
	EXCEPTION_TRY_GUARD

	int size = 256;
	char* buffer = 0;
	bool longEnough = false;
	while(!longEnough)
	{
		delete[] buffer;
		buffer = new char[size];
		va_list args;
		va_start( args, format );
#		ifdef _MSC_VER
		int len = _vsnprintf(buffer, size, format, args);
		// msdn says that _vsnprintf() returns a negative number if the buffer
		// wasn't long enough.  Add the extra (len < size) for safety in the
		// case MSVC becomes standard-compliant at some stage in the future...
		longEnough = len >= 0 && len < size;
		size *= 2;
#		else
		int len = vsnprintf(buffer, size, format, args);
		// According to the linux man pages, vsnprintf() returns a negative
		// value on error, or a positive value indicating the number of chars
		// which would have been written for an infinite-size buffer, not
		// including the terminating '\0'.  This is claimed to be the
		// C99-conforming behaviour.
		if(len < 0)
			return;
		longEnough = len < size;
		size = len+1;
#		endif
		va_end(args);
	}
	g_context->api().ArchiveRecord(type, buffer);

	delete[] buffer;
	EXCEPTION_CATCH_GUARD("ArchiveRecord")
}

/* vi: set et: */
