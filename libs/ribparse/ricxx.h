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

#ifndef AQSIS_RICXX_H_INCLUDED
#define AQSIS_RICXX_H_INCLUDED

#include <cassert>

#include <aqsis/riutil/primvartoken.h>
#include <aqsis/ri/ritypes.h>

namespace Aqsis {

/// Const versions of common Ri typedefs
typedef const float RtConstBasis[4][4];
typedef const float RtConstBound[6];
typedef const float RtConstColor[3];
typedef const float RtConstMatrix[4][4];
typedef const float RtConstPoint[3];
typedef const char* RtConstString;
typedef const char* RtConstToken;


namespace Ri {

typedef float RtFloat;
typedef int RtInt;
typedef const char* RtConstToken;

/// A minimal, immutable array reference type.
template<typename T>
class Array
{
    private:
        const T* m_data;
        size_t m_size;

    public:
        Array() : m_data(0), m_size(0) {}
        Array(const T* data, size_t size) : m_data(data), m_size(size) {}

        // Note, compiler generated assignment & copy construction.

        /// Iterators to beginning & end of array.
        const T* begin() const { return m_data; }
        const T* end() const { return m_data + m_size; }
        /// Get length of the array
        size_t size() const { return m_size; }

        /// Element access
        const T& operator[](size_t i) const { return m_data[i]; }
};

typedef Array<RtInt> IntArray;
typedef Array<RtFloat> FloatArray;
typedef Array<RtConstString> StringArray;
typedef Array<RtConstToken> TokenArray;

class Param
{
    private:
        CqPrimvarToken m_spec;
        const void* m_data;
        size_t m_size;

    public:
        Param(const CqPrimvarToken& spec, const RtFloat* data, size_t size)
            : m_spec(spec), m_data(data), m_size(size) {}
        Param(const CqPrimvarToken& spec, const RtInt* data, size_t size)
            : m_spec(spec), m_data(data), m_size(size) {}
        Param(const CqPrimvarToken& spec, const RtConstToken* data, size_t size)
            : m_spec(spec), m_data(data), m_size(size) {}
        template<typename T>
        Param(const CqPrimvarToken& spec, Array<T> value)
            : m_spec(spec), m_data(value.begin()), m_size(value.size()) {}

        const CqPrimvarToken& spec() const { return m_spec; }
        const void* data() const { return m_data; }

        FloatArray floatData() const
        {
            assert(m_spec.storageType() == type_float);
            return FloatArray(static_cast<const RtFloat*>(m_data), m_size);
        }
        IntArray intData() const
        {
            assert(m_spec.storageType() == type_int);
            return IntArray(static_cast<const RtInt*>(m_data), m_size);
        }
        StringArray stringData() const
        {
            assert(m_spec.storageType() == type_string);
            return StringArray(static_cast<const RtConstToken*>(m_data), m_size);
        }
};

typedef Array<Param> ParamList;


/// Simple C++ version of the RenderMan interface
///
/// The goal here is to provide a C++ version of the RenderMan interface which
/// is similar to the RIB binding.  This interface is *not* intended to be used
/// by end-users because it's fairly low-level and the call interface is not
/// particularly convenient.
///
/// The RIB binding is chosen as a model rather than the C binding, because it
/// naturally allows lengths to be passed along with all arrays to allow for
/// validation in the interface rather than only in the RIB parser.
///
/// Types are inspired by the C binding, but there are some differences because
/// we're trying to mirror the RIB binding, for convenience, and to fix some
/// const problems in the C API:
///
///   * Arrays are represented with the Array<T> template rather than with raw
///     C arrays.
///   * Parameter lists are passed as an array of Param structs; these provide
///     a richer more efficient format for specifying the value types rather
///     than the strings used in the C API.
///   * RtToken and RtString have been replaced by RtConstToken and
///     RtConstString - we're never going to modify these and modern C++
///     compilers complain bitterly when casting string literals to non-const
///     char*
///
class Renderer
{
    public:
        /// Bring Ri types into the Renderer class for convenience when
        /// implementing concrete Renderer classes outside the Ri namespace.
        typedef Ri::IntArray IntArray;
        typedef Ri::FloatArray FloatArray;
        typedef Ri::StringArray StringArray;
        typedef Ri::TokenArray TokenArray;
        typedef Ri::ParamList ParamList;

        // Autogenerated method declarations
        /*[[[cog
        import cog
        import sys, os
        sys.path.append(os.getcwd())
        from cogutils import *

        riXml = parseXmlTree('ri.xml')

        for p in riXml.findall('Procedures/' + '*'):
            if p.tag == 'Section':
                cog.outl(); cog.outl()
                cog.outl(commentBanner(p.text))
                continue
            if p.tag == 'SubSection':
                cog.outl(commentBanner(p.text, fillchar='-'))
                continue
            if p.tag == 'Procedure' and p.haschild('Rib'):
                decl = 'virtual %s = 0;' % (riCxxMethodDecl(p),)
                cog.outl(wrapDecl(decl, 72, wrapIndent=20))
        ]]]*/


        // ========== Relationship to the RenderMan Shading Language =========
        virtual RtToken Declare(RtConstString name,
                            RtConstString declaration) = 0;


        // ========================== Graphics State =========================
        virtual RtVoid FrameBegin(RtInt number) = 0;
        virtual RtVoid FrameEnd() = 0;
        virtual RtVoid WorldBegin() = 0;
        virtual RtVoid WorldEnd() = 0;
        // ------------------------- Conditional RIB -------------------------
        virtual RtVoid IfBegin(RtConstString condition) = 0;
        virtual RtVoid ElseIf(RtConstString condition) = 0;
        virtual RtVoid Else() = 0;
        virtual RtVoid IfEnd() = 0;
        // ----------------------------- Options -----------------------------
        virtual RtVoid Format(RtInt xresolution, RtInt yresolution,
                            RtFloat pixelaspectratio) = 0;
        virtual RtVoid FrameAspectRatio(RtFloat frameratio) = 0;
        virtual RtVoid ScreenWindow(RtFloat left, RtFloat right, RtFloat bottom,
                            RtFloat top) = 0;
        virtual RtVoid CropWindow(RtFloat xmin, RtFloat xmax, RtFloat ymin,
                            RtFloat ymax) = 0;
        virtual RtVoid Projection(RtConstToken name,
                            const ParamList& pList) = 0;
        virtual RtVoid Clipping(RtFloat cnear, RtFloat cfar) = 0;
        virtual RtVoid ClippingPlane(RtFloat x, RtFloat y, RtFloat z,
                            RtFloat nx, RtFloat ny, RtFloat nz) = 0;
        virtual RtVoid DepthOfField(RtFloat fstop, RtFloat focallength,
                            RtFloat focaldistance) = 0;
        virtual RtVoid Shutter(RtFloat opentime, RtFloat closetime) = 0;
        virtual RtVoid PixelVariance(RtFloat variance) = 0;
        virtual RtVoid PixelSamples(RtFloat xsamples, RtFloat ysamples) = 0;
        virtual RtVoid PixelFilter(RtFilterFunc function, RtFloat xwidth,
                            RtFloat ywidth) = 0;
        virtual RtVoid Exposure(RtFloat gain, RtFloat gamma) = 0;
        virtual RtVoid Imager(RtConstToken name, const ParamList& pList) = 0;
        virtual RtVoid Quantize(RtConstToken type, RtInt one, RtInt min,
                            RtInt max, RtFloat ditheramplitude) = 0;
        virtual RtVoid Display(RtConstToken name, RtConstToken type,
                            RtConstToken mode, const ParamList& pList) = 0;
        virtual RtVoid Hider(RtConstToken name, const ParamList& pList) = 0;
        virtual RtVoid ColorSamples(const FloatArray& nRGB,
                            const FloatArray& RGBn) = 0;
        virtual RtVoid RelativeDetail(RtFloat relativedetail) = 0;
        virtual RtVoid Option(RtConstToken name, const ParamList& pList) = 0;
        // ---------------------------- Attributes ---------------------------
        virtual RtVoid AttributeBegin() = 0;
        virtual RtVoid AttributeEnd() = 0;
        virtual RtVoid Color(RtConstColor Cq) = 0;
        virtual RtVoid Opacity(RtConstColor Os) = 0;
        virtual RtVoid TextureCoordinates(RtFloat s1, RtFloat t1, RtFloat s2,
                            RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4,
                            RtFloat t4) = 0;
        virtual RtLightHandle LightSource(RtConstToken name,
                            const ParamList& pList) = 0;
        virtual RtLightHandle AreaLightSource(RtConstToken name,
                            const ParamList& pList) = 0;
        virtual RtVoid Illuminate(RtLightHandle light, RtBoolean onoff) = 0;
        virtual RtVoid Surface(RtConstToken name, const ParamList& pList) = 0;
        virtual RtVoid Displacement(RtConstToken name,
                            const ParamList& pList) = 0;
        virtual RtVoid Atmosphere(RtConstToken name,
                            const ParamList& pList) = 0;
        virtual RtVoid Interior(RtConstToken name, const ParamList& pList) = 0;
        virtual RtVoid Exterior(RtConstToken name, const ParamList& pList) = 0;
        virtual RtVoid ShaderLayer(RtConstToken type, RtConstToken name,
                            RtConstToken layername,
                            const ParamList& pList) = 0;
        virtual RtVoid ConnectShaderLayers(RtConstToken type,
                            RtConstToken layer1, RtConstToken variable1,
                            RtConstToken layer2, RtConstToken variable2) = 0;
        virtual RtVoid ShadingRate(RtFloat size) = 0;
        virtual RtVoid ShadingInterpolation(RtConstToken type) = 0;
        virtual RtVoid Matte(RtBoolean onoff) = 0;
        virtual RtVoid Bound(RtConstBound bound) = 0;
        virtual RtVoid Detail(RtConstBound bound) = 0;
        virtual RtVoid DetailRange(RtFloat offlow, RtFloat onlow,
                            RtFloat onhigh, RtFloat offhigh) = 0;
        virtual RtVoid GeometricApproximation(RtConstToken type,
                            RtFloat value) = 0;
        virtual RtVoid Orientation(RtConstToken orientation) = 0;
        virtual RtVoid ReverseOrientation() = 0;
        virtual RtVoid Sides(RtInt nsides) = 0;
        // ------------------------- Transformations -------------------------
        virtual RtVoid Identity() = 0;
        virtual RtVoid Transform(RtConstMatrix transform) = 0;
        virtual RtVoid ConcatTransform(RtConstMatrix transform) = 0;
        virtual RtVoid Perspective(RtFloat fov) = 0;
        virtual RtVoid Translate(RtFloat dx, RtFloat dy, RtFloat dz) = 0;
        virtual RtVoid Rotate(RtFloat angle, RtFloat dx, RtFloat dy,
                            RtFloat dz) = 0;
        virtual RtVoid Scale(RtFloat sx, RtFloat sy, RtFloat sz) = 0;
        virtual RtVoid Skew(RtFloat angle, RtFloat dx1, RtFloat dy1,
                            RtFloat dz1, RtFloat dx2, RtFloat dy2,
                            RtFloat dz2) = 0;
        virtual RtVoid CoordinateSystem(RtConstToken space) = 0;
        virtual RtVoid CoordSysTransform(RtConstToken space) = 0;
        virtual RtVoid TransformBegin() = 0;
        virtual RtVoid TransformEnd() = 0;
        // ---------------------------- Resources ----------------------------
        virtual RtVoid Resource(RtConstToken handle, RtConstToken type,
                            const ParamList& pList) = 0;
        virtual RtVoid ResourceBegin() = 0;
        virtual RtVoid ResourceEnd() = 0;
        // ---------------- Implementation-specific Attributes ---------------
        virtual RtVoid Attribute(RtConstToken name,
                            const ParamList& pList) = 0;


        // ======================= Geometric Primitives ======================
        // ----------------------------- Polygons ----------------------------
        virtual RtVoid Polygon(const ParamList& pList) = 0;
        virtual RtVoid GeneralPolygon(const IntArray& nverts,
                            const ParamList& pList) = 0;
        virtual RtVoid PointsPolygons(const IntArray& nverts,
                            const IntArray& verts, const ParamList& pList) = 0;
        virtual RtVoid PointsGeneralPolygons(const IntArray& nloops,
                            const IntArray& nverts, const IntArray& verts,
                            const ParamList& pList) = 0;
        // ----------------------------- Patches -----------------------------
        virtual RtVoid Basis(RtConstBasis ubasis, RtInt ustep,
                            RtConstBasis vbasis, RtInt vstep) = 0;
        virtual RtVoid Patch(RtConstToken type, const ParamList& pList) = 0;
        virtual RtVoid PatchMesh(RtConstToken type, RtInt nu,
                            RtConstToken uwrap, RtInt nv, RtConstToken vwrap,
                            const ParamList& pList) = 0;
        virtual RtVoid NuPatch(RtInt nu, RtInt uorder, const FloatArray& uknot,
                            RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder,
                            const FloatArray& vknot, RtFloat vmin, RtFloat vmax,
                            const ParamList& pList) = 0;
        virtual RtVoid TrimCurve(const IntArray& ncurves, const IntArray& order,
                            const FloatArray& knot, const FloatArray& min,
                            const FloatArray& max, const IntArray& n,
                            const FloatArray& u, const FloatArray& v,
                            const FloatArray& w) = 0;
        // ----------------------- Subdivision Surfaces ----------------------
        virtual RtVoid SubdivisionMesh(RtConstToken scheme,
                            const IntArray& nvertices, const IntArray& vertices,
                            const TokenArray& tags, const IntArray& nargs,
                            const IntArray& intargs,
                            const FloatArray& floatargs,
                            const ParamList& pList) = 0;
        // ----------------------------- Quadrics ----------------------------
        virtual RtVoid Sphere(RtFloat radius, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList) = 0;
        virtual RtVoid Cone(RtFloat height, RtFloat radius, RtFloat thetamax,
                            const ParamList& pList) = 0;
        virtual RtVoid Cylinder(RtFloat radius, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList) = 0;
        virtual RtVoid Hyperboloid(RtConstPoint point1, RtConstPoint point2,
                            RtFloat thetamax, const ParamList& pList) = 0;
        virtual RtVoid Paraboloid(RtFloat rmax, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList) = 0;
        virtual RtVoid Disk(RtFloat height, RtFloat radius, RtFloat thetamax,
                            const ParamList& pList) = 0;
        virtual RtVoid Torus(RtFloat majorrad, RtFloat minorrad, RtFloat phimin,
                            RtFloat phimax, RtFloat thetamax,
                            const ParamList& pList) = 0;
        // ------------------- Points and Curve Primitives -------------------
        virtual RtVoid Points(const ParamList& pList) = 0;
        virtual RtVoid Curves(RtConstToken type, const IntArray& nvertices,
                            RtConstToken wrap, const ParamList& pList) = 0;
        // --------------------- Blobby Implicit Surfaces --------------------
        virtual RtVoid Blobby(RtInt nleaf, const IntArray& code,
                            const FloatArray& flt, const TokenArray& str,
                            const ParamList& pList) = 0;
        // ---------------------- Procedural Primitives ----------------------
        virtual RtVoid Procedural(RtPointer data, RtConstBound bound,
                            RtProcSubdivFunc refineproc,
                            RtProcFreeFunc freeproc) = 0;
        // ----------- Implementation-specific Geometric Primitives ----------
        virtual RtVoid Geometry(RtConstToken type, const ParamList& pList) = 0;
        // ----------------- Soids and Spatial Set Operations ----------------
        virtual RtVoid SolidBegin(RtConstToken type) = 0;
        virtual RtVoid SolidEnd() = 0;
        // ------------------------ Retained Geometry ------------------------
        virtual RtObjectHandle ObjectBegin() = 0;
        virtual RtVoid ObjectEnd() = 0;
        virtual RtVoid ObjectInstance(RtObjectHandle handle) = 0;


        // ============================== Motion =============================
        virtual RtVoid MotionBegin(const FloatArray& times) = 0;
        virtual RtVoid MotionEnd() = 0;


        // ======================== External Resources =======================
        // ---------------------- Texture Map Utilities ----------------------
        virtual RtVoid MakeTexture(RtConstString imagefile,
                            RtConstString texturefile, RtConstToken swrap,
                            RtConstToken twrap, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList) = 0;
        virtual RtVoid MakeLatLongEnvironment(RtConstString imagefile,
                            RtConstString reflfile, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList) = 0;
        virtual RtVoid MakeCubeFaceEnvironment(RtConstString px,
                            RtConstString nx, RtConstString py,
                            RtConstString ny, RtConstString pz,
                            RtConstString nz, RtConstString reflfile,
                            RtFloat fov, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList) = 0;
        virtual RtVoid MakeShadow(RtConstString picfile,
                            RtConstString shadowfile,
                            const ParamList& pList) = 0;
        virtual RtVoid MakeOcclusion(const StringArray& picfiles,
                            RtConstString shadowfile,
                            const ParamList& pList) = 0;
        // ------------------------------ Errors -----------------------------
        virtual RtVoid ErrorHandler(RtErrorFunc handler) = 0;
        // -------------------------- Archive Files --------------------------
        virtual RtVoid ReadArchive(RtConstToken name,
                            RtArchiveCallback callback,
                            const ParamList& pList) = 0;
        //[[[end]]]


        /// Oddball functions which are not part of the RIB binding, but are
        /// present for backward compatibility with the C API.
        virtual RtVoid ArchiveRecord(RtConstToken type, const char* string) = 0;

        /// Error handling callback, mainly for RIB parser.
        ///
        /// TODO: Think more about a cohesive error handling strategy.  Not
        /// sure if this should really be here or not.
        virtual RtVoid Error(const char* errorMessage) = 0;

        /// Functions returning pointers to standard filters, bases, etc.
        ///
        /// These are necessary so that (1) the interface can have control over
        /// what the standard bases etc. are, and (2) the parser doesn't need
        /// to provide symbols which resolve to these itself.
        virtual RtFilterFunc     GetFilterFunc(RtConstToken name) const = 0;
        virtual RtConstBasis*    GetBasis(RtConstToken name) const = 0;
        virtual RtErrorFunc      GetErrorFunc(RtConstToken name) const = 0;
        virtual RtProcSubdivFunc GetProcSubdivFunc(RtConstToken name) const = 0;
        virtual RtProcFreeFunc   GetProcFreeFunc() const = 0;

        // TODO: Rif API?
        //
        // TODO: Some way to get at the named transformation matrices so we can
        // implement RiTransformPoints

        virtual ~Renderer() {};
};


} // namespace Ri
} // namespace Aqsis

#endif // AQSIS_RICXX_H_INCLUDED
// vi: set et:
