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
/// \brief Utilities to make working with the Ri types nicer
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///

#ifndef AQSIS_RICXXUTIL_H_INCLUDED
#define AQSIS_RICXXUTIL_H_INCLUDED

#include <aqsis/riutil/ricxx.h>

#include <cassert>
#include <climits>
#include <string.h> // for strcmp

#include <aqsis/riutil/primvartoken.h>
#include <aqsis/riutil/interpclasscounts.h>
#include <aqsis/util/exception.h>

namespace Aqsis {

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
        /// Add a single value, v to the parameter list
        template<typename T>
        ParamListBuilder& operator()(const char* token, T* v);
        /// Add a single value, v to the parameter list
        template<typename T>
        ParamListBuilder& operator()(const Ri::TypeSpec& spec,
                                     const char* name, T* v);

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
};


//--------------------------------------------------
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
SqInterpClassCounts patchMeshIClassCounts(const char* type, int nu,
                                          const char* uwrap,
                                          int nv, const char* vwrap,
                                          int basisUstep, int basisVstep);

/// Get the interpolation class counts for RiCurves.
SqInterpClassCounts curvesIClassCounts(const char* type,
                                       const Ri::IntArray& nvertices,
                                       const char* wrap, int basisVstep);


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

        virtual void parseRib(std::istream& ribStream, const char* name,
                              Ri::Renderer& context) { }

        using Ri::RendererServices::parseRib;
};


//==============================================================================
// implementation details

template<typename T>
inline ParamListBuilder& ParamListBuilder::operator()(const char* token, T* v)
{
    const char* nameBegin = 0;
    const char* nameEnd = 0;
    Ri::TypeSpec spec = parseDeclaration(token, &nameBegin, &nameEnd);
    assert(*nameEnd == '\0');
    m_paramStorage.push_back(
        Ri::Param(spec, nameBegin, Ri::Array<T>(v, 1)));
    return *this;
}

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
inline ParamListBuilder& ParamListBuilder::operator()(const Ri::TypeSpec& spec,
                                               const char* name, T* v)
{
    m_paramStorage.push_back(
        Ri::Param(spec, name, Ri::Array<T>(v, 1)));
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
                                   int nv, const char* vwrap, int basisUstep, int basisVstep)
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
    else
    {
        int nupatches = uperiodic ? nu/basisUstep : (nu-4)/basisUstep + 1;
        int nvpatches = vperiodic ? nv/basisVstep : (nv-4)/basisVstep + 1;
        iclassCounts.uniform = nupatches * nvpatches;
        iclassCounts.varying = ((uperiodic ? 0 : 1) + nupatches) *
                               ((vperiodic ? 0 : 1) + nvpatches);
    }
    iclassCounts.vertex = nu*nv;
    // TODO: are facevertex/facevarying valid for a patch mesh?
    iclassCounts.facevarying = 1; //iclassCounts.uniform*4; //??
    iclassCounts.facevertex = 1;
    return iclassCounts;
}

inline SqInterpClassCounts curvesIClassCounts(const char* type,
                                       const Ri::IntArray& nvertices,
                                       const char* wrap, int basisVstep)
{
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    bool periodic = strcmp(wrap, "periodic") == 0;
    int basisStep = basisVstep;
    iclassCounts.uniform = size(nvertices);
    iclassCounts.vertex = sum(nvertices);
    if(strcmp(type, "cubic") == 0)
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
