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
/// \brief Realization of the RI in C++ objects
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///

#ifndef AQSIS_RICXX_H_INCLUDED
#define AQSIS_RICXX_H_INCLUDED

#include <cassert>
#include <iosfwd>
#include <stddef.h> // for size_t
#include <string.h> // for strcmp

#include <aqsis/config.h>
#include <aqsis/ri/ritypes.h>

/// Const versions of common Ri typedefs.
///
/// In the global namespace to match RtFloat etc.  (Is this a good idea?)
typedef const float RtConstBasis[4][4];
typedef const float RtConstBound[6];
typedef const float RtConstColor[3];
typedef const float RtConstMatrix[4][4];
typedef const float RtConstPoint[3];
typedef const char* RtConstString;
typedef const char* RtConstToken;


namespace Aqsis {

/// Namespace containing a C++ version of the RI, with associated types.
namespace Ri {

// Setup.

//------------------------------------------------------------------------------
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

        /// Conversion to void* for boolean context testing
        operator const void*() const { return m_data; }
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
typedef Array<void*> PtrArray;


//------------------------------------------------------------------------------
/// Type specification for parameters.
///
/// In the traditional RI, interface types are represented using strings, for
/// example "point", "uniform vector[1]", etc.  This struct is a non-string
/// representation the three important pieces of information contained in such
/// type strings.
///   * Interpolation class - the method used for interpolating the value
///     across the surface of a geometric primitive
///   * Type - the basic type.
///   * Array size - variables may be arrays of one or more elements of the
///     basic type.  Non-array variables are treated as arrays with length 1.
struct TypeSpec
{
    // note: order of IClass and Type should match the Rif interface,
    // RifTokenDetail and RifTokenType.
    enum IClass
    {
        Constant,
        Uniform,
        Varying,
        Vertex,
        FaceVarying,
        FaceVertex,
        NoClass = 128
    };
    enum Type
    {
        Float,
        Point,
        Color,
        Integer,
        String,
        Vector,
        Normal,
        HPoint,
        Matrix,
        MPoint,
        Pointer,
        Unknown = 128,
        Int=Integer,
    };

    IClass iclass;
    Type type;
    int arraySize;

    TypeSpec(Type type=Unknown, int arraySize=1)
        : iclass(Uniform), type(type), arraySize(arraySize) {}
    TypeSpec(IClass iclass, Type type, int arraySize=1)
        : iclass(iclass), type(type), arraySize(arraySize) {}

    /// Return the base type used for storage of this type.
    ///
    /// The current type is an aggregate of elements of the base type.
    Type storageType() const
    {
        switch(type)
        {
            case Integer: case String:
            case Unknown: case Pointer:    return type;
            default:                       return Float;
        }
    }

    /// Return number of values of the base type needed per element.
    ///
    /// This depends on the type, and the array count.  For example, a vector
    /// uses 3 floats per element, so the type vector[2] would need 3*2 = 6
    /// float values per element.
    ///
    /// \param numColComps - The number of color components.
    int storageCount(int nColComps = 3) const
    {
        int typeCount = 0;
        switch(type)
        {
            case Float: case Integer: case String: typeCount = 1;  break;
            case Pointer:                          typeCount = 1;  break;
            case Point: case Normal:  case Vector: typeCount = 3;  break;
            case Color:                            typeCount = nColComps; break;
            case HPoint:                           typeCount = 4;  break;
            case Matrix: case MPoint:              typeCount = 16; break;
            default:
                assert(0 && "storage length unknown for type"); break;
        }
        return typeCount*arraySize;
    }
};

/// Equality testing for TypeSpec's
inline bool operator==(const TypeSpec& lhs, const TypeSpec& rhs)
{
    return lhs.iclass == rhs.iclass && lhs.type == rhs.type &&
           lhs.arraySize == rhs.arraySize;
}
/// Compare TypeSpec to "uniform type[1]"
inline bool operator==(const TypeSpec& lhs, TypeSpec::Type type)
{
    return lhs == TypeSpec(type);
}
/// Compare TypeSpec to "uniform type[1]"
inline bool operator==(TypeSpec::Type type, const TypeSpec& rhs)
{
    return TypeSpec(type) == rhs;
}

/// Metafunction converting from a C++ type to a TypeSpec storage type
template<typename T> struct toTypeSpecType { };
#define AQSIS_DEFINE_TO_TYPESPEC(cppType, tsType)              \
template<> struct toTypeSpecType<cppType>                      \
    { static const TypeSpec::Type value = TypeSpec::tsType; }
AQSIS_DEFINE_TO_TYPESPEC(RtFloat, Float);
AQSIS_DEFINE_TO_TYPESPEC(RtInt, Integer);
AQSIS_DEFINE_TO_TYPESPEC(RtConstString, String);
AQSIS_DEFINE_TO_TYPESPEC(void*, Pointer);
#undef AQSIS_DEFINE_TO_TYPESPEC

//------------------------------------------------------------------------------
/// Representation of a parameter for interface function parameter lists
class Param
{
    private:
        TypeSpec m_spec;
        const char* m_name;
        const void* m_data;
        size_t m_size;

    public:
        Param() : m_spec(), m_name(""), m_data(0), m_size(0) {}
        Param(const TypeSpec& spec, const char* name, const RtFloat* data, size_t size)
            : m_spec(spec), m_name(name), m_data(data), m_size(size) {}
        Param(const TypeSpec& spec, const char* name, const RtInt* data, size_t size)
            : m_spec(spec), m_name(name), m_data(data), m_size(size) {}
        Param(const TypeSpec& spec, const char* name, const RtConstToken* data, size_t size)
            : m_spec(spec), m_name(name), m_data(data), m_size(size) {}
        Param(const TypeSpec& spec, const char* name, const void* data, size_t size)
            : m_spec(spec), m_name(name), m_data(data), m_size(size) {}
        template<typename T>
        Param(const TypeSpec& spec, const char* name, Array<T> value)
            : m_spec(spec), m_name(name), m_data(value.begin()), m_size(value.size()) {}

        const TypeSpec& spec() const { return m_spec; }
        const char* name() const { return m_name; }
        const void* data() const { return m_data; }
        size_t size() const { return m_size; }

        template<typename T>
        Ri::Array<T> data() const
        {
            assert(m_spec.storageType() == toTypeSpecType<T>::value);
            return Ri::Array<T>(static_cast<const T*>(m_data), m_size);
        }
        FloatArray floatData()   const { return data<RtFloat>(); }
        IntArray   intData()     const { return data<RtInt>(); }
        StringArray stringData() const { return data<RtConstString>(); }
        PtrArray   ptrData()     const { return data<void*>(); }
};


/// A parameter list of type/name, value pairs.
///
/// This is just an Array of Ri::Param, augmented with a small amount of extra
/// functionality for looking up parameters, etc.
class ParamList : public Array<Param>
{
    public:
        ParamList() : Array<Param>() {}
        ParamList(const Param* data, size_t size) : Array<Param>(data, size) {}

        /// Find a parameter with the given type and name in the list.
        ///
        /// \return the index of the found parameter, or -1 if it wasn't found
        /// in the list.
        int find(const TypeSpec& spec, const char* name) const
        {
            for(size_t i = 0; i < size(); ++i)
            {
                const Param& p = (*this)[i];
                if(p.spec() == spec && !strcmp(p.name(), name))
                    return i;
            }
            return -1;
        }

        /// Find a parameter with the given type and name.
        ///
        /// \return An array of data corresponding to the value of the
        /// parameter.  A null array is returned if the parameter isn't found,
        /// or the storage type is incompatible with the type T.
        ///
        /// TODO: Think about whether other types could be supported here - eg,
        /// it might be nice to be able to do
        ///
        /// pList.find<Imath::V3f>(TypeSpec::Vector, "P");
        template<typename T>
        Array<T> find(const TypeSpec& spec, const char* name) const
        {
            if(spec.storageType() != toTypeSpecType<T>::value)
                return Array<T>();
            int idx = find(spec, name);
            if(idx < 0)
                return Array<T>();
            else
                return (*this)[idx].data<T>();
        }

        /// Find value of a "uniform type[1]" parameter with the given name.
        FloatArray findFloat(const char* name) const
            { return find<RtFloat>(TypeSpec::Float, name); }
        FloatArray findPoint(const char* name) const
            { return find<RtFloat>(TypeSpec::Point, name); }
        FloatArray findColor(const char* name) const
            { return find<RtFloat>(TypeSpec::Color, name); }
        FloatArray findVector(const char* name) const
            { return find<RtFloat>(TypeSpec::Vector, name); }
        FloatArray findNormal(const char* name) const
            { return find<RtFloat>(TypeSpec::Normal, name); }
        FloatArray findHPoint(const char* name) const
            { return find<RtFloat>(TypeSpec::HPoint, name); }
        FloatArray findMatrix(const char* name) const
            { return find<RtFloat>(TypeSpec::Matrix, name); }
        FloatArray findMPoint(const char* name) const
            { return find<RtFloat>(TypeSpec::MPoint, name); }
        IntArray findInt(const char* name) const
            { return find<RtInt>(TypeSpec::Int, name); }
        StringArray findString(const char* name) const
            { return find<RtConstString>(TypeSpec::String, name); }
        PtrArray findPtr(const char* name) const
            { return find<void*>(TypeSpec::Pointer, name); }
};


//------------------------------------------------------------------------------
/// Simple C++ version of the RenderMan interface
///
/// The goal here is to provide a C++ version of the RenderMan interface which
/// is similar to the RIB binding.  This interface is not intended to be used
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
class AQSIS_RIUTIL_SHARE Renderer
{
    public:
        /// Bring Ri types into the Renderer class for convenience when
        /// implementing concrete Renderer classes outside the Ri namespace.
        typedef Ri::IntArray IntArray;
        typedef Ri::FloatArray FloatArray;
        typedef Ri::StringArray StringArray;
        typedef Ri::TokenArray TokenArray;
        typedef Ri::ParamList ParamList;
        typedef Ri::TypeSpec TypeSpec;

        // FIXME: Declare should return RtConstToken.

        // Autogenerated method declarations
        /*[[[cog
        from codegenutils import *

        riXml = parseXml(riXmlPath)

        for p in riXml.findall('Procedures/' + '*'):
            if p.tag == 'Section':
                cog.outl(); cog.outl()
                cog.outl(commentBanner(p.text))
                continue
            if p.tag == 'SubSection':
                cog.outl(commentBanner(p.text, fillchar='-'))
                continue
            if p.tag == 'Procedure' and p.findall('Rib'):
                decl = 'virtual %s = 0;' % (riCxxMethodDecl(p, useDefaults=True),)
                cog.outl(wrapDecl(decl, 72, wrapIndent=20))
        ]]]*/


        // ========== Relationship to the RenderMan Shading Language =========
        virtual RtVoid Declare(RtConstString name,
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
                            const ParamList& pList = ParamList()) = 0;
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
        virtual RtVoid Imager(RtConstToken name,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid Quantize(RtConstToken type, RtInt one, RtInt min,
                            RtInt max, RtFloat ditheramplitude) = 0;
        virtual RtVoid Display(RtConstToken name, RtConstToken type,
                            RtConstToken mode,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid Hider(RtConstToken name,
                            const ParamList& pList = ParamList()) = 0;
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
        virtual RtVoid LightSource(RtConstToken shadername, RtConstToken name,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid AreaLightSource(RtConstToken shadername,
                            RtConstToken name,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid Illuminate(RtConstToken name, RtBoolean onoff) = 0;
        virtual RtVoid Surface(RtConstToken name,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid Displacement(RtConstToken name,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid Atmosphere(RtConstToken name,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid Interior(RtConstToken name,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid Exterior(RtConstToken name,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid ShaderLayer(RtConstToken type, RtConstToken name,
                            RtConstToken layername,
                            const ParamList& pList = ParamList()) = 0;
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
                            RtFloat thetamax,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid Cone(RtFloat height, RtFloat radius, RtFloat thetamax,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid Cylinder(RtFloat radius, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid Hyperboloid(RtConstPoint point1, RtConstPoint point2,
                            RtFloat thetamax,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid Paraboloid(RtFloat rmax, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid Disk(RtFloat height, RtFloat radius, RtFloat thetamax,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid Torus(RtFloat majorrad, RtFloat minorrad, RtFloat phimin,
                            RtFloat phimax, RtFloat thetamax,
                            const ParamList& pList = ParamList()) = 0;
        // ------------------- Points and Curve Primitives -------------------
        virtual RtVoid Points(const ParamList& pList) = 0;
        virtual RtVoid Curves(RtConstToken type, const IntArray& nvertices,
                            RtConstToken wrap, const ParamList& pList) = 0;
        // --------------------- Blobby Implicit Surfaces --------------------
        virtual RtVoid Blobby(RtInt nleaf, const IntArray& code,
                            const FloatArray& floats, const TokenArray& strings,
                            const ParamList& pList = ParamList()) = 0;
        // ---------------------- Procedural Primitives ----------------------
        virtual RtVoid Procedural(RtPointer data, RtConstBound bound,
                            RtProcSubdivFunc refineproc,
                            RtProcFreeFunc freeproc) = 0;
        // ----------- Implementation-specific Geometric Primitives ----------
        virtual RtVoid Geometry(RtConstToken type,
                            const ParamList& pList = ParamList()) = 0;
        // ----------------- Soids and Spatial Set Operations ----------------
        virtual RtVoid SolidBegin(RtConstToken type) = 0;
        virtual RtVoid SolidEnd() = 0;
        // ------------------------ Retained Geometry ------------------------
        virtual RtVoid ObjectBegin(RtConstToken name) = 0;
        virtual RtVoid ObjectEnd() = 0;
        virtual RtVoid ObjectInstance(RtConstToken name) = 0;


        // ============================== Motion =============================
        virtual RtVoid MotionBegin(const FloatArray& times) = 0;
        virtual RtVoid MotionEnd() = 0;


        // ======================== External Resources =======================
        // ---------------------- Texture Map Utilities ----------------------
        virtual RtVoid MakeTexture(RtConstString imagefile,
                            RtConstString texturefile, RtConstToken swrap,
                            RtConstToken twrap, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid MakeLatLongEnvironment(RtConstString imagefile,
                            RtConstString reflfile, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid MakeCubeFaceEnvironment(RtConstString px,
                            RtConstString nx, RtConstString py,
                            RtConstString ny, RtConstString pz,
                            RtConstString nz, RtConstString reflfile,
                            RtFloat fov, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid MakeShadow(RtConstString picfile,
                            RtConstString shadowfile,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid MakeOcclusion(const StringArray& picfiles,
                            RtConstString shadowfile,
                            const ParamList& pList = ParamList()) = 0;
        // ------------------------------ Errors -----------------------------
        virtual RtVoid ErrorHandler(RtErrorFunc handler) = 0;
        // -------------------------- Archive Files --------------------------
        virtual RtVoid ReadArchive(RtConstToken name,
                            RtArchiveCallback callback,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid ArchiveBegin(RtConstToken name,
                            const ParamList& pList = ParamList()) = 0;
        virtual RtVoid ArchiveEnd() = 0;
        //[[[end]]]

        /// ArchiveRecord isn't part of the RIB binding, but a variant of it
        /// here is included here for convenience in passing RIB comments.
        virtual RtVoid ArchiveRecord(RtConstToken type, const char* string) = 0;

        virtual ~Renderer() {};
};

class ErrorHandler;
class Filter;

/// Access to extra renderer state, interface filters, RIB parsing etc.
///
/// Roughly speaking, this class is a collection of the parts of Ri::Renderer
/// which aren't filterable.  It's also a convenient place to manage filter
/// chains from.
class RendererServices
{
    public:
        /// Get Error handler.
        virtual ErrorHandler& errorHandler() = 0;

        /// Functions returning pointers to standard filters, bases, etc.
        ///
        /// These are necessary so that (1) the interface can have control over
        /// what the standard bases etc. are, and (2) the parser doesn't need
        /// to provide symbols which resolve to these itself.
        virtual RtFilterFunc     getFilterFunc(RtConstToken name) const = 0;
        virtual RtConstBasis*    getBasis(RtConstToken name) const = 0;
        virtual RtErrorFunc      getErrorFunc(RtConstToken name) const = 0;
        virtual RtProcSubdivFunc getProcSubdivFunc(RtConstToken name) const = 0;

        /// Get a previously Declared token or parse inline declaration
        ///
        /// The parameter name is a subrange of the string "token", and as such
        /// is returned as the range [nameBegin, nameEnd).
        ///
        /// Implementations should throw XqValidation on parse or lookup error.
        ///
        /// \param token - type declaration and parameter name, in a string.
        /// \param nameBegin - return start of the name here if non-null
        /// \param nameEnd   - return end of the name here if non-null
        virtual TypeSpec getDeclaration(RtConstToken token,
                                        const char** nameBegin = 0,
                                        const char** nameEnd = 0) const = 0;

        /// Get first filter in the chain, or the underlying renderer.
        ///
        /// If there is no filters currently attached, return the Renderer
        /// instance associated with this RendererServices object.
        virtual Renderer& firstFilter() = 0;

        /// Add a filter to the front of the filter chain
        ///
        /// throw XqValidation when the filter can't be found.
        virtual void addFilter(const char* name,
                               const ParamList& filterParams = ParamList()) = 0;

        /// Add a filter to the front of the filter chain.
        ///
        /// The lifetime of the provided filter is assumed to be controlled by
        /// the caller - the filter lifetime *must* outlast the lifetime of the
        /// renderer services object.
        virtual void addFilter(Ri::Filter& filter) = 0;

        /// Parse a RIB stream
        ///
        /// Insert the resulting stream of commands into the given Renderer
        /// context.
        ///
        /// \param ribStream - input RIB stream
        /// \param name - name of RIB stream (for debugging purposes)
        /// \param context - sink for parsed commands
        virtual void parseRib(std::istream& ribStream, const char* name,
                              Renderer& context) = 0;

        /// Parse a RIB stream using the first filter in the chain.
        virtual void parseRib(std::istream& ribStream, const char* name)
            { parseRib(ribStream, name, firstFilter()); }

        virtual ~RendererServices() {}
};


} // namespace Ri
} // namespace Aqsis

#endif // AQSIS_RICXX_H_INCLUDED
// vi: set et:
