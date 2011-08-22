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
/// \brief Utilities to make working with the Ri types nicer
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///

#ifndef AQSIS_RICXXUTIL_H_INCLUDED
#define AQSIS_RICXXUTIL_H_INCLUDED

#include <aqsis/riutil/ricxx.h>

#include <cassert>
#include <climits>
#include <deque>
#include <string.h> // for strcmp

#include <aqsis/riutil/primvartoken.h>
#include <aqsis/riutil/interpclasscounts.h>
#include <aqsis/util/exception.h>

#include <boost/function.hpp>

namespace Aqsis {

//------------------------------------------------------------------------------
/// A class for convenient building of Ri::ParamList instances
///
/// Example:
/// \code
///     float dist = 0;
///     const char* name = "a string";
///     std::vector<int> frames(10);
///
///     // Construct the paramer list (this can also be done inline)
///     ParamListBuilder pList();
///     pList("float dist", &dist)
///          (Ri::TypeSpec(Ri::TypeSpec::String, "name", &name)
///          ("int frames", frames);
///
///     // Can assign to a ParamList, or pass to a function taking one such
///     Ri::ParamList realList = pList;
/// \endcode
///
class ParamListBuilder
{
    public:
        /// Add a "uniform T[1]" to the parameter list
        template<typename T>
        ParamListBuilder& operator()(const char* name, T v);

        /// Add an array, v to the parameter list
        template<typename T>
        ParamListBuilder& operator()(const char* token,
                                     const std::vector<T>& v);
        /// Add an array, v to the parameter list
        template<typename T>
        ParamListBuilder& operator()(const Ri::TypeSpec& spec,
                                     const char* name,
                                     const std::vector<T>& v);
        /// Add a parameter to the list
        template<typename T>
        ParamListBuilder& operator()(const Ri::Param& p);


        /// Implicity convert to Ri::ParamList
        operator Ri::ParamList();

    private:
        std::vector<Ri::Param> m_paramStorage;

        /// Union to store types which have been provided by value so we can
        /// take pointers to them to put in an Ri::Array<T>
        union SingleValue
        {
            RtInt i;
            RtFloat f;
            RtConstString s;
            const void* p;
            SingleValue(RtInt i)         : i(i) {}
            SingleValue(RtFloat f)       : f(f) {}
            SingleValue(RtConstString s) : s(s) {}
            SingleValue(const void* p)   : p(p) {}
        };
        /// Use a deque here rather than a vector so that references to the
        /// held values aren't invalidated when the deque expands via push_back
        std::deque<SingleValue> m_valueStorage;
};


//------------------------------------------------------------------------------
/// Class for making sure that all parameters in a param list are used.
///
/// As each parameter is extracted from the list using one of the find()
/// methods, the parameter is marked as used.  The usued parameters can then
/// be extracted and the user warned that they are ignored.
class ParamListUsage
{
    public:
        ParamListUsage(const Ri::ParamList& pList);

        /// Find a parameter, and mark it as used.
        ///
        /// \see Ri::ParamList::find
        template<typename T>
        Ri::Array<T> find(const Ri::TypeSpec& spec, const char* name);

        /// Find a uniform parameter, and mark it as used.
        Ri::FloatArray findFloat(const char* name)
            { return find<RtFloat>(Ri::TypeSpec::Float, name); }
        Ri::FloatArray findPoint(const char* name)
            { return find<RtFloat>(Ri::TypeSpec::Point, name); }
        Ri::FloatArray findColor(const char* name)
            { return find<RtFloat>(Ri::TypeSpec::Color, name); }
        Ri::FloatArray findVector(const char* name)
            { return find<RtFloat>(Ri::TypeSpec::Vector, name); }
        Ri::FloatArray findNormal(const char* name)
            { return find<RtFloat>(Ri::TypeSpec::Normal, name); }
        Ri::FloatArray findHPoint(const char* name)
            { return find<RtFloat>(Ri::TypeSpec::HPoint, name); }
        Ri::FloatArray findMatrix(const char* name)
            { return find<RtFloat>(Ri::TypeSpec::Matrix, name); }
        Ri::FloatArray findMPoint(const char* name)
            { return find<RtFloat>(Ri::TypeSpec::MPoint, name); }
        Ri::IntArray findInt(const char* name)
            { return find<RtInt>(Ri::TypeSpec::Int, name); }
        Ri::StringArray findString(const char* name)
            { return find<RtConstString>(Ri::TypeSpec::String, name); }
        Ri::PtrArray findPtr(const char* name)
            { return find<void*>(Ri::TypeSpec::Pointer, name); }

        /// Return true if some parameters are yet to be used via find*()
        bool hasUnusedParams();

        /// Return a string containing the unhandled parameter names
        ///
        /// Since this is designed for error reporting a simple string is used
        /// rather than a vector of strings.
        std::string unusedParams();

    private:
        const Ri::ParamList& m_pList;
        std::vector<bool> m_handled;
};


//------------------------------------------------------------------------------
/// Get the array size from counts corresponding to iclass.
int iclassCount(const SqInterpClassCounts& counts,
                       Ri::TypeSpec::IClass iclass);

/// Add elements of a
int sum(const Ri::IntArray& a);

/// Add elements of a, with given starting index and stride
int sum(const Ri::IntArray& a, int start, int step);

/// Get the maximum element in array a
int max(const Ri::IntArray& a);

/// Get the length of the array a
template<typename T>
int size(const Ri::Array<T>& a);

/// Count the length of the "P" array in the parameter list
inline int countP(const Ri::ParamList& pList);

/// Get the interpolation class counts for RiPatchMesh.
///
/// If basisKnown is false, insert -1 for any counts which depend on the basis
/// step.
SqInterpClassCounts patchMeshIClassCounts(const char* type, int nu,
                                          const char* uwrap,
                                          int nv, const char* vwrap,
                                          int basisUstep, int basisVstep,
                                          bool basisKnown = true);

/// Get the interpolation class counts for RiCurves.
///
/// If basisKnown is false, insert -1 for any counts which depend on the basis
/// step.
SqInterpClassCounts curvesIClassCounts(const char* type,
                                       const Ri::IntArray& nvertices,
                                       const char* wrap, int basisVstep,
                                       bool basisKnown = true);


//------------------------------------------------------------------------------
typedef boost::function<bool(const char*)> IfElseTestCallback;

/// Create a "Renderer utility filter"
///
/// This is a filter which contains parts of the standard interface handling
/// which can be conveniently autogenerated:  inline archive/object instancing
/// and ifbegin/elseif/else/ifend handling.
///
/// The filter caches all calls between ArchiveBegin and ArchiveEnd into
/// memory.  Whenever a ReadArchive call with the same name is processed, we
/// insert the contents of the cached stream back into the first filter in the
/// chain.
///
/// Some PRMan docs found online suggest that inline archives may be
/// arbitrarily nested, so we allow this behaviour by keeping track of the
/// archive nesting level.
///
/// The object instancing mechanism is so similar to inline archive handling
/// that we use the same machinary for both.
///
/// Conditional RIB handling is also performed, before the archive and object
/// steps handling steps.  The callback provided should take a condition
/// string, and return a bool indicating whether the condition evaluated to
/// true or false.
///
AQSIS_RIUTIL_SHARE
Ri::Filter* createRenderUtilFilter(const IfElseTestCallback& callback =
                                   IfElseTestCallback());

//------------------------------------------------------------------------------
/// Empty implementation of Ri::Renderer
///
/// This is a do-nothing implementation of Ri::Renderer for convenience, in the
/// case that a user doesn't want to implement all the renderer methods.
class StubRenderer : public Ri::Renderer
{
    public:
        /*[[[cog
        from codegenutils import *
        riXml = parseXml(riXmlPath)
        for p in riXml.findall('Procedures/Procedure'):
            if p.findall('Rib'):
                decl = 'virtual %s {}' % (riCxxMethodDecl(p),)
                cog.outl(wrapDecl(decl, 72, wrapIndent=20))
        ]]]*/
        virtual RtVoid Declare(RtConstString name,
                            RtConstString declaration) {}
        virtual RtVoid FrameBegin(RtInt number) {}
        virtual RtVoid FrameEnd() {}
        virtual RtVoid WorldBegin() {}
        virtual RtVoid WorldEnd() {}
        virtual RtVoid IfBegin(RtConstString condition) {}
        virtual RtVoid ElseIf(RtConstString condition) {}
        virtual RtVoid Else() {}
        virtual RtVoid IfEnd() {}
        virtual RtVoid Format(RtInt xresolution, RtInt yresolution,
                            RtFloat pixelaspectratio) {}
        virtual RtVoid FrameAspectRatio(RtFloat frameratio) {}
        virtual RtVoid ScreenWindow(RtFloat left, RtFloat right, RtFloat bottom,
                            RtFloat top) {}
        virtual RtVoid CropWindow(RtFloat xmin, RtFloat xmax, RtFloat ymin,
                            RtFloat ymax) {}
        virtual RtVoid Projection(RtConstToken name, const ParamList& pList) {}
        virtual RtVoid Clipping(RtFloat cnear, RtFloat cfar) {}
        virtual RtVoid ClippingPlane(RtFloat x, RtFloat y, RtFloat z,
                            RtFloat nx, RtFloat ny, RtFloat nz) {}
        virtual RtVoid DepthOfField(RtFloat fstop, RtFloat focallength,
                            RtFloat focaldistance) {}
        virtual RtVoid Shutter(RtFloat opentime, RtFloat closetime) {}
        virtual RtVoid PixelVariance(RtFloat variance) {}
        virtual RtVoid PixelSamples(RtFloat xsamples, RtFloat ysamples) {}
        virtual RtVoid PixelFilter(RtFilterFunc function, RtFloat xwidth,
                            RtFloat ywidth) {}
        virtual RtVoid Exposure(RtFloat gain, RtFloat gamma) {}
        virtual RtVoid Imager(RtConstToken name, const ParamList& pList) {}
        virtual RtVoid Quantize(RtConstToken type, RtInt one, RtInt min,
                            RtInt max, RtFloat ditheramplitude) {}
        virtual RtVoid Display(RtConstToken name, RtConstToken type,
                            RtConstToken mode, const ParamList& pList) {}
        virtual RtVoid Hider(RtConstToken name, const ParamList& pList) {}
        virtual RtVoid ColorSamples(const FloatArray& nRGB,
                            const FloatArray& RGBn) {}
        virtual RtVoid RelativeDetail(RtFloat relativedetail) {}
        virtual RtVoid Option(RtConstToken name, const ParamList& pList) {}
        virtual RtVoid AttributeBegin() {}
        virtual RtVoid AttributeEnd() {}
        virtual RtVoid Color(RtConstColor Cq) {}
        virtual RtVoid Opacity(RtConstColor Os) {}
        virtual RtVoid TextureCoordinates(RtFloat s1, RtFloat t1, RtFloat s2,
                            RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4,
                            RtFloat t4) {}
        virtual RtVoid LightSource(RtConstToken shadername, RtConstToken name,
                            const ParamList& pList) {}
        virtual RtVoid AreaLightSource(RtConstToken shadername,
                            RtConstToken name, const ParamList& pList) {}
        virtual RtVoid Illuminate(RtConstToken name, RtBoolean onoff) {}
        virtual RtVoid Surface(RtConstToken name, const ParamList& pList) {}
        virtual RtVoid Displacement(RtConstToken name,
                            const ParamList& pList) {}
        virtual RtVoid Atmosphere(RtConstToken name, const ParamList& pList) {}
        virtual RtVoid Interior(RtConstToken name, const ParamList& pList) {}
        virtual RtVoid Exterior(RtConstToken name, const ParamList& pList) {}
        virtual RtVoid ShaderLayer(RtConstToken type, RtConstToken name,
                            RtConstToken layername, const ParamList& pList) {}
        virtual RtVoid ConnectShaderLayers(RtConstToken type,
                            RtConstToken layer1, RtConstToken variable1,
                            RtConstToken layer2, RtConstToken variable2) {}
        virtual RtVoid ShadingRate(RtFloat size) {}
        virtual RtVoid ShadingInterpolation(RtConstToken type) {}
        virtual RtVoid Matte(RtBoolean onoff) {}
        virtual RtVoid Bound(RtConstBound bound) {}
        virtual RtVoid Detail(RtConstBound bound) {}
        virtual RtVoid DetailRange(RtFloat offlow, RtFloat onlow,
                            RtFloat onhigh, RtFloat offhigh) {}
        virtual RtVoid GeometricApproximation(RtConstToken type,
                            RtFloat value) {}
        virtual RtVoid Orientation(RtConstToken orientation) {}
        virtual RtVoid ReverseOrientation() {}
        virtual RtVoid Sides(RtInt nsides) {}
        virtual RtVoid Identity() {}
        virtual RtVoid Transform(RtConstMatrix transform) {}
        virtual RtVoid ConcatTransform(RtConstMatrix transform) {}
        virtual RtVoid Perspective(RtFloat fov) {}
        virtual RtVoid Translate(RtFloat dx, RtFloat dy, RtFloat dz) {}
        virtual RtVoid Rotate(RtFloat angle, RtFloat dx, RtFloat dy,
                            RtFloat dz) {}
        virtual RtVoid Scale(RtFloat sx, RtFloat sy, RtFloat sz) {}
        virtual RtVoid Skew(RtFloat angle, RtFloat dx1, RtFloat dy1,
                            RtFloat dz1, RtFloat dx2, RtFloat dy2,
                            RtFloat dz2) {}
        virtual RtVoid CoordinateSystem(RtConstToken space) {}
        virtual RtVoid CoordSysTransform(RtConstToken space) {}
        virtual RtVoid TransformBegin() {}
        virtual RtVoid TransformEnd() {}
        virtual RtVoid Resource(RtConstToken handle, RtConstToken type,
                            const ParamList& pList) {}
        virtual RtVoid ResourceBegin() {}
        virtual RtVoid ResourceEnd() {}
        virtual RtVoid Attribute(RtConstToken name, const ParamList& pList) {}
        virtual RtVoid Polygon(const ParamList& pList) {}
        virtual RtVoid GeneralPolygon(const IntArray& nverts,
                            const ParamList& pList) {}
        virtual RtVoid PointsPolygons(const IntArray& nverts,
                            const IntArray& verts, const ParamList& pList) {}
        virtual RtVoid PointsGeneralPolygons(const IntArray& nloops,
                            const IntArray& nverts, const IntArray& verts,
                            const ParamList& pList) {}
        virtual RtVoid Basis(RtConstBasis ubasis, RtInt ustep,
                            RtConstBasis vbasis, RtInt vstep) {}
        virtual RtVoid Patch(RtConstToken type, const ParamList& pList) {}
        virtual RtVoid PatchMesh(RtConstToken type, RtInt nu,
                            RtConstToken uwrap, RtInt nv, RtConstToken vwrap,
                            const ParamList& pList) {}
        virtual RtVoid NuPatch(RtInt nu, RtInt uorder, const FloatArray& uknot,
                            RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder,
                            const FloatArray& vknot, RtFloat vmin, RtFloat vmax,
                            const ParamList& pList) {}
        virtual RtVoid TrimCurve(const IntArray& ncurves, const IntArray& order,
                            const FloatArray& knot, const FloatArray& min,
                            const FloatArray& max, const IntArray& n,
                            const FloatArray& u, const FloatArray& v,
                            const FloatArray& w) {}
        virtual RtVoid SubdivisionMesh(RtConstToken scheme,
                            const IntArray& nvertices, const IntArray& vertices,
                            const TokenArray& tags, const IntArray& nargs,
                            const IntArray& intargs,
                            const FloatArray& floatargs,
                            const ParamList& pList) {}
        virtual RtVoid Sphere(RtFloat radius, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList) {}
        virtual RtVoid Cone(RtFloat height, RtFloat radius, RtFloat thetamax,
                            const ParamList& pList) {}
        virtual RtVoid Cylinder(RtFloat radius, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList) {}
        virtual RtVoid Hyperboloid(RtConstPoint point1, RtConstPoint point2,
                            RtFloat thetamax, const ParamList& pList) {}
        virtual RtVoid Paraboloid(RtFloat rmax, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList) {}
        virtual RtVoid Disk(RtFloat height, RtFloat radius, RtFloat thetamax,
                            const ParamList& pList) {}
        virtual RtVoid Torus(RtFloat majorrad, RtFloat minorrad, RtFloat phimin,
                            RtFloat phimax, RtFloat thetamax,
                            const ParamList& pList) {}
        virtual RtVoid Points(const ParamList& pList) {}
        virtual RtVoid Curves(RtConstToken type, const IntArray& nvertices,
                            RtConstToken wrap, const ParamList& pList) {}
        virtual RtVoid Blobby(RtInt nleaf, const IntArray& code,
                            const FloatArray& floats, const TokenArray& strings,
                            const ParamList& pList) {}
        virtual RtVoid Procedural(RtPointer data, RtConstBound bound,
                            RtProcSubdivFunc refineproc,
                            RtProcFreeFunc freeproc) {}
        virtual RtVoid Geometry(RtConstToken type, const ParamList& pList) {}
        virtual RtVoid SolidBegin(RtConstToken type) {}
        virtual RtVoid SolidEnd() {}
        virtual RtVoid ObjectBegin(RtConstToken name) {}
        virtual RtVoid ObjectEnd() {}
        virtual RtVoid ObjectInstance(RtConstToken name) {}
        virtual RtVoid MotionBegin(const FloatArray& times) {}
        virtual RtVoid MotionEnd() {}
        virtual RtVoid MakeTexture(RtConstString imagefile,
                            RtConstString texturefile, RtConstToken swrap,
                            RtConstToken twrap, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList) {}
        virtual RtVoid MakeLatLongEnvironment(RtConstString imagefile,
                            RtConstString reflfile, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList) {}
        virtual RtVoid MakeCubeFaceEnvironment(RtConstString px,
                            RtConstString nx, RtConstString py,
                            RtConstString ny, RtConstString pz,
                            RtConstString nz, RtConstString reflfile,
                            RtFloat fov, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList) {}
        virtual RtVoid MakeShadow(RtConstString picfile,
                            RtConstString shadowfile,
                            const ParamList& pList) {}
        virtual RtVoid MakeOcclusion(const StringArray& picfiles,
                            RtConstString shadowfile,
                            const ParamList& pList) {}
        virtual RtVoid ErrorHandler(RtErrorFunc handler) {}
        virtual RtVoid ReadArchive(RtConstToken name,
                            RtArchiveCallback callback,
                            const ParamList& pList) {}
        virtual RtVoid ArchiveBegin(RtConstToken name,
                            const ParamList& pList) {}
        virtual RtVoid ArchiveEnd() {}
        ///[[[end]]]

        virtual RtVoid ArchiveRecord(RtConstToken type, const char* string) {}
};


/// Empty partial implementation of Ri::RendererServices.
///
/// This is included for convenience, in the case that a quick and dirty
/// implementation without all the functionality is required.
///
/// To make a full non-virtual subclass, you need to implement:
///   * errorHandler()
///   * getDeclaration()
///   * firstFilter()
class StubRendererServices : public Ri::RendererServices
{
    public:
        virtual RtFilterFunc     getFilterFunc(RtConstToken name) const { return 0; }
        virtual RtConstBasis*    getBasis(RtConstToken name) const { return 0; }
        virtual RtErrorFunc      getErrorFunc(RtConstToken name) const { return 0; }
        virtual RtProcSubdivFunc getProcSubdivFunc(RtConstToken name) const { return 0; }

        virtual void addFilter(const char* name,
                               const Ri::ParamList& filterParams) { }
        virtual void addFilter(Ri::Filter& filter) { }

        virtual void parseRib(std::istream& ribStream, const char* name,
                              Ri::Renderer& context) { }

        using Ri::RendererServices::parseRib;
};


//==============================================================================
// implementation details

inline ParamListUsage::ParamListUsage(const Ri::ParamList& pList)
    : m_pList(pList),
    m_handled(pList.size(), false)
{ }

template<typename T>
inline Ri::Array<T> ParamListUsage::find(const Ri::TypeSpec& spec,
                                           const char* name)
{
    assert(spec.storageType() == Ri::toTypeSpecType<T>::value);
    int idx = m_pList.find(spec, name);
    if(idx < 0)
        return Ri::Array<T>();
    m_handled[idx] = true;
    return m_pList[idx].data<T>();
}

inline bool ParamListUsage::hasUnusedParams()
{
    for(int i = 0; i < (int)m_handled.size(); ++i)
        if(!m_handled[i])
            return true;
    return false;
}

inline std::string ParamListUsage::unusedParams()
{
    std::string unhandled;
    for(int i = 0; i < (int)m_handled.size(); ++i)
    {
        if(!m_handled[i])
        {
            if(!unhandled.empty())
                unhandled += ", ";
            unhandled += m_pList[i].name();
        }
    }
    return unhandled;
}


//--------------------------------------------------
template<typename T>
inline ParamListBuilder& ParamListBuilder::operator()(const char* token,
                                               const std::vector<T>& v)
{
    const char* nameBegin = 0;
    const char* nameEnd = 0;
    Ri::TypeSpec spec = parseDeclaration(token, &nameBegin, &nameEnd);
    assert(*nameEnd == '\0');
    m_paramStorage.push_back(
        Ri::Param(spec, nameBegin,
                    Ri::Array<T>(v.empty() ? 0 : &v[0], v.size())));
    return *this;
}

template<typename T>
ParamListBuilder& ParamListBuilder::operator()(const char* name, T v)
{
    m_valueStorage.push_back(SingleValue(v));
    m_paramStorage.push_back(Ri::Param(Ri::toTypeSpecType<T>::value, name,
                        static_cast<const void*>(&m_valueStorage.back()), 1));
    return *this;
}

template<typename T>
inline ParamListBuilder& ParamListBuilder::operator()(const Ri::TypeSpec& spec,
                                               const char* name,
                                               const std::vector<T>& v)
{
    m_paramStorage.push_back(
        Ri::Param(spec, name,
                    Ri::Array<T>(v.empty() ? 0 : &v[0], v.size())));
    return *this;
}

template<typename T>
ParamListBuilder& ParamListBuilder::operator()(const Ri::Param& p)
{
    m_paramStorage.push_back(p);
    return *this;
}

inline ParamListBuilder::operator Ri::ParamList()
{
    if(m_paramStorage.empty())
        return Ri::ParamList();
    return Ri::ParamList(&m_paramStorage[0], m_paramStorage.size());
}

//--------------------------------------------------
inline int iclassCount(const SqInterpClassCounts& counts,
                       Ri::TypeSpec::IClass iclass)
{
    switch(iclass)
    {
        case Ri::TypeSpec::Constant:    return 1;
        case Ri::TypeSpec::Uniform:     return counts.uniform;
        case Ri::TypeSpec::Varying:     return counts.varying;
        case Ri::TypeSpec::Vertex:      return counts.vertex;
        case Ri::TypeSpec::FaceVarying: return counts.facevarying;
        case Ri::TypeSpec::FaceVertex:  return counts.facevertex;
        default:
            assert(0 && "Unknown interpolation class"); return 0;
    }
}

inline int sum(const Ri::IntArray& a)
{
    int s = 0;
    for(size_t i = 0; i < a.size(); ++i)
        s += a[i];
    return s;
}

inline int sum(const Ri::IntArray& a, int start, int step)
{
    int s = 0;
    for(size_t i = start; i < a.size(); i+=step)
        s += a[i];
    return s;
}

inline int max(const Ri::IntArray& a)
{
    if(a.size() == 0)
        return 0;
    int m = INT_MIN;
    for(size_t i = 0; i < a.size(); ++i)
        if(m < a[i])
            m = a[i];
    return m;
}

template<typename T>
inline int size(const Ri::Array<T>& a)
{
    return a.size();
}

inline int countP(const Ri::ParamList& pList)
{
    for(size_t i = 0; i < pList.size(); ++i)
    {
        if(!strcmp(pList[i].name(), "P"))
            return pList[i].size()/3;
        if(!strcmp(pList[i].name(), "Pw"))
            return pList[i].size()/4;
    }
    AQSIS_THROW_XQERROR(XqValidation, EqE_MissingData,
            "\"P\" not found in parameter list");
    return -1;
}


//--------------------------------------------------
inline SqInterpClassCounts patchMeshIClassCounts(const char* type, int nu, const char* uwrap,
                                   int nv, const char* vwrap, int basisUstep, int basisVstep,
                                   bool basisKnown)
{
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    bool uperiodic = strcmp(uwrap, "periodic") == 0;
    bool vperiodic = strcmp(vwrap, "periodic") == 0;
    if(strcmp(type, "bilinear")==0)
    {
        iclassCounts.uniform = (uperiodic ? nu : nu-1) *
                               (vperiodic ? nv : nv-1);
        iclassCounts.varying = nu*nv;
    }
    else // bicubic
    {
        if(basisKnown)
        {
            int nupatches = uperiodic ? nu/basisUstep : (nu-4)/basisUstep + 1;
            int nvpatches = vperiodic ? nv/basisVstep : (nv-4)/basisVstep + 1;
            iclassCounts.uniform = nupatches * nvpatches;
            iclassCounts.varying = ((uperiodic ? 0 : 1) + nupatches) *
                                ((vperiodic ? 0 : 1) + nvpatches);
        }
        else
        {
            iclassCounts.uniform = -1;
            iclassCounts.varying = -1;
        }
    }
    iclassCounts.vertex = nu*nv;
    // TODO: are facevertex/facevarying valid for a patch mesh?
    iclassCounts.facevarying = 1; //iclassCounts.uniform*4; //??
    iclassCounts.facevertex = 1;
    return iclassCounts;
}

inline SqInterpClassCounts curvesIClassCounts(const char* type,
                                       const Ri::IntArray& nvertices,
                                       const char* wrap, int basisVstep,
                                       bool basisKnown)
{
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    bool periodic = strcmp(wrap, "periodic") == 0;
    int basisStep = basisVstep;
    iclassCounts.uniform = size(nvertices);
    iclassCounts.vertex = sum(nvertices);
    if(strcmp(type, "cubic") == 0)
    {
        if(basisKnown)
        {
            if(periodic)
            {
                int segmentCount = 0;
                for(size_t i = 0; i < nvertices.size(); ++i)
                    segmentCount += nvertices[i]/basisStep;
                iclassCounts.varying = segmentCount;
            }
            else
            {
                int segmentCount = 0;
                for(size_t i = 0; i < nvertices.size(); ++i)
                    segmentCount += (nvertices[i]-4)/basisStep + 1;
                iclassCounts.varying = segmentCount + size(nvertices);
            }
        }
        else
            iclassCounts.varying = -1;
    }
    else
    {
        // linear curves
        iclassCounts.varying = iclassCounts.vertex;
    }
    // TODO: are facevertex/facevarying valid for curves?
    iclassCounts.facevarying = 1;
    iclassCounts.facevertex = 1;
    return iclassCounts;
}



}

#endif // AQSIS_RICXXUTIL_H_INCLUDED
// vi: set et:
