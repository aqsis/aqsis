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
/// \brief RIB parser tests
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///

#include <aqsis/aqsis.h>

#define BOOST_TEST_DYN_LINK

#include "ribparser_impl.h"

#include <cfloat>
#include <cstdlib>
#include <sstream>

#include <boost/assign/std/vector.hpp>
#include <boost/test/auto_unit_test.hpp>

#include <aqsis/math/math.h>
#include <aqsis/riutil/tokendictionary.h>
#include <aqsis/riutil/errorhandler.h>

using namespace boost::assign; // necessary for container initialisation operators.

using namespace Aqsis;

//------------------------------------------------------------------------------
// Test setup.  Unfortunately we need a LOT of this :(
//------------------------------------------------------------------------------
namespace {
// struct wrapping a request name for use in parser parameter vector.
struct Req
{
    std::string name;

    Req() : name() {}
    explicit Req(const char* name) : name(name) {}
    Req(const Req& rq) : name(rq.name) {}

    bool operator==(const Req& rhs) const
    {
        return name == rhs.name;
    }
};

namespace printer_funcs
{
    template<typename T>
    void ribTokenPrint(std::ostream& out, const T& v) { out << v; }

    // Wrap strings in token lists with a pair of " chars
    void ribTokenPrint(std::ostream& out, const std::string& s)
    {
        out << '"' << s << '"';
    }

    // Printer functions for use with boost.test test tools
    // We put these operator<<() in a namespace so that they can be introduced
    // into namespace boost in a controllable way.

    // Insert a std::vector into a stream in RIB format.
    template<typename T>
    std::ostream& operator<<(std::ostream& out, const std::vector<T>& v)
    {
        out << "[";
        for(int i = 0, end = v.size(); i < end; ++i)
        {
            ribTokenPrint(out, v[i]);
            if(i != end-1)
                out << " ";
        }
        out << "]";
        return out;
    }

    // Print a Ri::Array
    template<typename T>
    std::ostream& operator<<(std::ostream& out, const Ri::Array<T>& v)
    {
        out << "[";
        for(int i = 0, end = v.size(); i < end; ++i)
        {
            ribTokenPrint(out, v[i]);
            if(i != end-1)
                out << " ";
        }
        out << "]";
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const Ri::Param& p)
    {
        out << CqPrimvarToken(p.spec(), p.name());
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const Ri::TypeSpec& spec)
    {
        CqPrimvarToken tok(spec, "");
        out << tok.Class() << " " << tok.type();
        if(tok.count() != 1)
            out << "[" << tok.count() << "]";
        return out;
    }

    // Insert a Req into a stream.
    std::ostream& operator<<(std::ostream& out, const Req& r)
    {
        out << r.name;
        return out;
    }
}

// Introduce the printer funcs into the necessary namespaces so that lookup can
// find them (ugh, is there a better way to do this?)  One alternative would be
// to just disable printing using something like
//
//   BOOST_TEST_DONT_PRINT_LOG_VALUE(Ri::Param);
//
} // anon. namespace 

namespace Aqsis { namespace Ri {
    using printer_funcs::operator<<;
}}

// Note that BOOST_AUTO_TEST_SUITE uses a namespace as implementation, so we
// need to take care when mixing it with our own namespaces.
BOOST_AUTO_TEST_SUITE(rib_parser_tests)

namespace {
using printer_funcs::operator<<;

//------------------------------------------------------------------------------
// Types for MockRibToken
enum MockTokType
{
    Type_Int,
    Type_Float,
    Type_String,
    Type_IntArray,
    Type_FloatArray,
    Type_StringArray,
    Type_Request,
    Type_Ignore
};

// Tag struct to indicate an empty MockRibToken, used to reconcile differing
// RIB and RI forms of requests where necessary.
struct IgnoreType {};
static IgnoreType IgnoreParam;


// Cheap and nasty variant holding RIB tokens
//
// Yeah, boost::variant would allow me to avoid implementing this class, but
// the error messages are absolutely insane, to the point of making the code
// unmaintainable.
//
struct MockRibToken
{
private:
    MockTokType m_type;

    int m_int;
    float m_float;
    std::string m_string;
    Req m_request;
    std::vector<int> m_ints;
    std::vector<float> m_floats;
    std::vector<std::string> m_strings;

public:
    explicit MockRibToken(int i)                    : m_type(Type_Int), m_int(i) { }
    explicit MockRibToken(float f)                  : m_type(Type_Float), m_float(f) { }
    explicit MockRibToken(const char* s)            : m_type(Type_String), m_string(s) { }
    explicit MockRibToken(const std::string& s)     : m_type(Type_String), m_string(s) { }
    explicit MockRibToken(const Req& request)       : m_type(Type_Request), m_request(request) { }
    explicit MockRibToken(const std::vector<int>& v)         : m_type(Type_IntArray), m_ints(v) { }
    explicit MockRibToken(const std::vector<float>& v)       : m_type(Type_FloatArray), m_floats(v) { }
    explicit MockRibToken(const std::vector<std::string>& v) : m_type(Type_StringArray), m_strings(v) { }
    explicit MockRibToken(const Ri::IntArray& v)    : m_type(Type_IntArray), m_ints(v.begin(), v.end()) { }
    explicit MockRibToken(const Ri::FloatArray& v)  : m_type(Type_FloatArray), m_floats(v.begin(), v.end()) { }
    explicit MockRibToken(const Ri::StringArray& v) : m_type(Type_StringArray), m_strings(v.begin(), v.end()) { }
    explicit MockRibToken(const IgnoreType&)        : m_type(Type_Ignore) { }

    MockTokType type() const { return m_type; }

    int getInt() const                   { BOOST_REQUIRE(m_type == Type_Int); return m_int; }
    float getFloat() const               { BOOST_REQUIRE(m_type == Type_Float); return m_float; }
    const std::string& getString() const { BOOST_REQUIRE(m_type == Type_String); return m_string; }
    const Req& getReq() const            { BOOST_REQUIRE(m_type == Type_Request); return m_request; }
    const std::vector<int>& getIntArray() const            { BOOST_REQUIRE(m_type == Type_IntArray); return m_ints; }
    const std::vector<float>& getFloatArray() const        { BOOST_REQUIRE(m_type == Type_FloatArray); return m_floats; }
    const std::vector<std::string>& getStringArray() const { BOOST_REQUIRE(m_type == Type_StringArray); return m_strings; }

    bool operator==(const MockRibToken& rhs) const
    {
        if(m_type == Type_Ignore)
        {
            // Always consider an ignored param to be "equal" to whatever
            // we feed in.  This is for convenience when comparing RIB and
            // RI forms of a request.
            return true;
        }
        if(m_type != rhs.m_type)
            return false;
        switch(m_type)
        {
            case Type_Int:         return getInt() == rhs.getInt();
            case Type_Float:       return getFloat() == rhs.getFloat();
            case Type_String:      return getString() == rhs.getString();
            case Type_IntArray:    return getIntArray() == rhs.getIntArray();
            case Type_FloatArray:  return getFloatArray() == rhs.getFloatArray();
            case Type_StringArray: return getStringArray() == rhs.getStringArray();
            case Type_Request:     return getReq() == rhs.getReq();
            case Type_Ignore:      return true;
        }
        BOOST_REQUIRE(0 && "unrecognised m_type??");
        return false;
    }
};

typedef std::vector<MockRibToken> TokenVec;

std::ostream& operator<<(std::ostream& out, const MockRibToken& tok)
{
//    if(&out != &std::cerr)
//        std::cerr << "_X" << tok.type() << tok << "X_";
    switch(tok.type())
    {
        case Type_Int:          out << tok.getInt();        break;
        case Type_Float:        out << tok.getFloat();      break;
        case Type_String:       out << '"' << tok.getString() << '"'; break;
        case Type_IntArray:     out << tok.getIntArray();   break;
        case Type_FloatArray:   out << tok.getFloatArray(); break;
        case Type_StringArray:  out << tok.getStringArray();break;
        case Type_Request:      out << tok.getReq().name;   break;
        case Type_Ignore:                                   break;
    }
    return out;
}

// Compare the contents of a std::vector to an Ri::Array
template<typename T1, typename T2>
bool operator==(const std::vector<T1>& v1, const Ri::Array<T2>& v2)
{
    if(v1.size() != v2.size())
        return false;
    for(int i = 0, end = v1.size(); i < end; ++i)
    {
        if(v1[i] != v2[i])
            return false;
    }
    return true;
}

// Compare a token containing an array to an Ri::Param array
//
// assert if the token doesn't contain an array.
bool operator==(const MockRibToken& lhs, const Ri::Param& rhs)
{
    switch(lhs.type())
    {
        case Type_IntArray:    return lhs.getIntArray() == rhs.intData();
        case Type_FloatArray:  return lhs.getFloatArray() == rhs.floatData();
        case Type_StringArray: return lhs.getStringArray() == rhs.stringData();
        default:
            BOOST_CHECK_EQUAL(lhs.type(), MockTokType(100)); // hack to print type.
            BOOST_REQUIRE(0 && "MockRibToken Ri::Param compare not implemented for type");
    }
    return false;
}


// Exception to throw from MockRenderer::Error
struct MockRendererError
{
    std::string message;
    MockRendererError(const std::string& msg) : message(msg) {}
};

// Some dummy symbols returned by MockRenderer.
RtConstBasis g_mockBasis = {{1,0,0,0}, {0,2,0,0}, {0,0,3,0}, {0,0,0,4}};

RtVoid mockProcSubdivFunc( RtPointer data, RtFloat detail )
{}
RtFloat mockFilterFunc(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth)
{
    return 1;
}

// Mock implementation of Ri::Renderer.
class MockRenderer : public Ri::Renderer
{
    public:
        MockRenderer() {}

        // Define all methods, except those which are tested.
        /*[[[cog
        from codegenutils import *

        riXml = parseXml(riXmlPath)

        # Here's all methods which are currently tested:
        testedMethods = set([
            'Declare',
            'DepthOfField',
            'PixelFilter',
            'ColorSamples',
            'Option',
            'Color',
            'LightSource',
            'Illuminate',
            'Transform',
            'Basis',
            'SubdivisionMesh',
            'Sphere',
            'Hyperboloid',
            'Points',
            'Procedural',
            'ObjectBegin',
            'MotionBegin',
            'MakeOcclusion',
        ])

        for p in riXml.findall('Procedures/Procedure'):
            if p.findall('Rib'):
                decl = riCxxMethodDecl(p)
                procName = p.findtext('Name')
                if procName in testedMethods:
                    cog.outl('// %s is tested' % (procName,))
                    decl = 'virtual %s;' % (decl,)
                else:
                    decl = 'virtual %s {}' % (decl,)
                cog.outl(wrapDecl(decl, 72, wrapIndent=20))
        ]]]*/
        // Declare is tested
        virtual RtVoid Declare(RtConstString name, RtConstString declaration);
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
        // DepthOfField is tested
        virtual RtVoid DepthOfField(RtFloat fstop, RtFloat focallength,
                            RtFloat focaldistance);
        virtual RtVoid Shutter(RtFloat opentime, RtFloat closetime) {}
        virtual RtVoid PixelVariance(RtFloat variance) {}
        virtual RtVoid PixelSamples(RtFloat xsamples, RtFloat ysamples) {}
        // PixelFilter is tested
        virtual RtVoid PixelFilter(RtFilterFunc function, RtFloat xwidth,
                            RtFloat ywidth);
        virtual RtVoid Exposure(RtFloat gain, RtFloat gamma) {}
        virtual RtVoid Imager(RtConstToken name, const ParamList& pList) {}
        virtual RtVoid Quantize(RtConstToken type, RtInt one, RtInt min,
                            RtInt max, RtFloat ditheramplitude) {}
        virtual RtVoid Display(RtConstToken name, RtConstToken type,
                            RtConstToken mode, const ParamList& pList) {}
        virtual RtVoid Hider(RtConstToken name, const ParamList& pList) {}
        // ColorSamples is tested
        virtual RtVoid ColorSamples(const FloatArray& nRGB,
                            const FloatArray& RGBn);
        virtual RtVoid RelativeDetail(RtFloat relativedetail) {}
        // Option is tested
        virtual RtVoid Option(RtConstToken name, const ParamList& pList);
        virtual RtVoid AttributeBegin() {}
        virtual RtVoid AttributeEnd() {}
        // Color is tested
        virtual RtVoid Color(RtConstColor Cq);
        virtual RtVoid Opacity(RtConstColor Os) {}
        virtual RtVoid TextureCoordinates(RtFloat s1, RtFloat t1, RtFloat s2,
                            RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4,
                            RtFloat t4) {}
        // LightSource is tested
        virtual RtVoid LightSource(RtConstToken shadername, RtConstToken name,
                            const ParamList& pList);
        virtual RtVoid AreaLightSource(RtConstToken shadername,
                            RtConstToken name, const ParamList& pList) {}
        // Illuminate is tested
        virtual RtVoid Illuminate(RtConstToken name, RtBoolean onoff);
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
        // Transform is tested
        virtual RtVoid Transform(RtConstMatrix transform);
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
        // Basis is tested
        virtual RtVoid Basis(RtConstBasis ubasis, RtInt ustep,
                            RtConstBasis vbasis, RtInt vstep);
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
        // SubdivisionMesh is tested
        virtual RtVoid SubdivisionMesh(RtConstToken scheme,
                            const IntArray& nvertices, const IntArray& vertices,
                            const TokenArray& tags, const IntArray& nargs,
                            const IntArray& intargs,
                            const FloatArray& floatargs,
                            const ParamList& pList);
        // Sphere is tested
        virtual RtVoid Sphere(RtFloat radius, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList);
        virtual RtVoid Cone(RtFloat height, RtFloat radius, RtFloat thetamax,
                            const ParamList& pList) {}
        virtual RtVoid Cylinder(RtFloat radius, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList) {}
        // Hyperboloid is tested
        virtual RtVoid Hyperboloid(RtConstPoint point1, RtConstPoint point2,
                            RtFloat thetamax, const ParamList& pList);
        virtual RtVoid Paraboloid(RtFloat rmax, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList) {}
        virtual RtVoid Disk(RtFloat height, RtFloat radius, RtFloat thetamax,
                            const ParamList& pList) {}
        virtual RtVoid Torus(RtFloat majorrad, RtFloat minorrad, RtFloat phimin,
                            RtFloat phimax, RtFloat thetamax,
                            const ParamList& pList) {}
        // Points is tested
        virtual RtVoid Points(const ParamList& pList);
        virtual RtVoid Curves(RtConstToken type, const IntArray& nvertices,
                            RtConstToken wrap, const ParamList& pList) {}
        virtual RtVoid Blobby(RtInt nleaf, const IntArray& code,
                            const FloatArray& floats, const TokenArray& strings,
                            const ParamList& pList) {}
        // Procedural is tested
        virtual RtVoid Procedural(RtPointer data, RtConstBound bound,
                            RtProcSubdivFunc refineproc,
                            RtProcFreeFunc freeproc);
        virtual RtVoid Geometry(RtConstToken type, const ParamList& pList) {}
        virtual RtVoid SolidBegin(RtConstToken type) {}
        virtual RtVoid SolidEnd() {}
        // ObjectBegin is tested
        virtual RtVoid ObjectBegin(RtConstToken name);
        virtual RtVoid ObjectEnd() {}
        virtual RtVoid ObjectInstance(RtConstToken name) {}
        // MotionBegin is tested
        virtual RtVoid MotionBegin(const FloatArray& times);
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
        // MakeOcclusion is tested
        virtual RtVoid MakeOcclusion(const StringArray& picfiles,
                            RtConstString shadowfile, const ParamList& pList);
        virtual RtVoid ErrorHandler(RtErrorFunc handler) {}
        virtual RtVoid ReadArchive(RtConstToken name,
                            RtArchiveCallback callback,
                            const ParamList& pList) {}
        virtual RtVoid ArchiveBegin(RtConstToken name,
                            const ParamList& pList) {}
        virtual RtVoid ArchiveEnd() {}
        //[[[end]]]

        virtual RtVoid ArchiveRecord(RtConstToken type, const char* string) {}
};

/// An error handler which throws MockRendererError
class MockErrorHandler : public Ri::ErrorHandler
{
    public:
        MockErrorHandler() : ErrorHandler(Warning) { }

    protected:
        virtual void dispatch(int code, const std::string& message)
        {
            throw MockRendererError(message);
        }
};

// Mock implementation of Ri::RendererServices
class MockServices : public Ri::RendererServices
{
    private:
        TokenDict m_tokenDict;
        MockErrorHandler m_errorHandler;

        friend RtVoid MockRenderer::Declare(RtConstString name,
                RtConstString declaration);
    public:
        virtual ErrorHandler& errorHandler() { return m_errorHandler; }
        virtual RtFilterFunc     getFilterFunc(RtConstToken name) const {return &mockFilterFunc;}
        virtual RtConstBasis*    getBasis(RtConstToken name) const {return &g_mockBasis;}
        virtual RtErrorFunc      getErrorFunc(RtConstToken name) const {return 0;}
        virtual RtProcSubdivFunc getProcSubdivFunc(RtConstToken name) const {return &mockProcSubdivFunc;}

        virtual Ri::TypeSpec getDeclaration(RtConstToken token,
                                const char** nameBegin = 0,
                                const char** nameEnd = 0) const
        {
            return m_tokenDict.lookup(token, nameBegin, nameEnd);
        }

        void addFilter(const char* name, const Ri::ParamList& filterParams)
        { }
        virtual void addFilter(Ri::Filter& filter)
        { }

        Ri::Renderer& firstFilter()
        {
            // Wow, how awful!
            return *(Ri::Renderer*)0;
        }

        void parseRib(std::istream& ribStream, const char* name,
                      Ri::Renderer& context)
        { }
};


struct Fixture;
static Fixture* g_fixture = 0;

// Fixture for all tests.
//
// Insert tokens into the RIB stream using operator<<
struct Fixture
{
    MockRenderer renderer;
    MockServices services;
    TokenVec tokens;
    RibParserImpl parser;  //< to test.
    int checkPos;

    Fixture(int checkPosOffset = 0)
        : renderer(),
        tokens(),
        parser(services),
        checkPos(checkPosOffset)
    {
        g_fixture = this;
    }

    ~Fixture()
    {
        std::stringstream ribStream;
        for(int i = 0; i < (int)tokens.size(); ++i)
            ribStream << tokens[i] << " ";
        parser.parseStream(ribStream, "test_stream", renderer);
        BOOST_CHECK_EQUAL(checkPos, (int)tokens.size());
        g_fixture = 0;
    }

    template<typename T>
    Fixture& operator<<(const T& tok)
    {
        tokens.push_back(MockRibToken(tok));
        return *this;
    }
};

struct StringOrInt
{
    const char* str;
    StringOrInt(const char* str) : str(str) {}
};

// A checker for request parameters
//
// This uses the tokens stored g_fixture to check the parameters which are
// passed out into an interface call.
struct CheckParams
{
    private:
        // Dictionary of standard tokens.
        static TokenDict m_tokenDict;

        void nextParam() { ++g_fixture->checkPos; }

    public:
        CheckParams()
        {
            BOOST_REQUIRE(g_fixture);
        }

        CheckParams& operator<<(const IgnoreType&)
        {
            nextParam();
            return *this;
        }
        template<typename T>
        CheckParams& operator<<(const T& tok)
        {
            BOOST_CHECK_EQUAL(g_fixture->tokens.at(g_fixture->checkPos),
                              MockRibToken(tok));
            nextParam();
            return *this;
        }
        CheckParams& operator<<(const StringOrInt tok)
        {
            const MockRibToken t = g_fixture->tokens.at(g_fixture->checkPos);
            if(t.type() == Type_Int)
                BOOST_CHECK_EQUAL(MockRibToken(std::atoi(tok.str)), t);
            else
                BOOST_CHECK_EQUAL(MockRibToken(tok.str), t);
            nextParam();
            return *this;
        }
        CheckParams& operator<<(const Ri::ParamList& pList)
        {
            for(int i = 0; i < (int)pList.size(); ++i)
            {
                try
                {
                    const char* name = 0;
                    Ri::TypeSpec spec = m_tokenDict.lookup(
                            g_fixture->tokens.at(g_fixture->checkPos)
                            .getString().c_str(), &name);
                    BOOST_CHECK_EQUAL(spec, pList[i].spec());
                    BOOST_CHECK_EQUAL(name, pList[i].name());
                }
                catch(XqValidation& /*e*/)
                {
                    // Intentionally blank for now, since it's tricky to test
                    // the custom-defined tokens.
                }
                nextParam();
                BOOST_CHECK_EQUAL(g_fixture->tokens.at(g_fixture->checkPos),
                                  pList[i]);
                nextParam();
            }
            return *this;
        }
};
TokenDict CheckParams::m_tokenDict;

} // anon. namespace

//------------------------------------------------------------------------------
// Actual test cases
//------------------------------------------------------------------------------
// Tests for handler functions with hand-written implementations.

BOOST_AUTO_TEST_CASE(RIB_version_test)
{
    // Test the RIB-only version token... this doesn't actually do anything
    // right now but shouldn't generate an error
    Fixture(2) << Req("version") << 3.03f;
}

//--------------------------------------------------
RtVoid MockRenderer::Declare(RtConstString name, RtConstString declaration)
{
    CheckParams() << Req("Declare") << name << declaration;
    // UGH!
    g_fixture->services.m_tokenDict.declare(name, declaration);
}
BOOST_AUTO_TEST_CASE(Declare_handler_test)
{
    // (Note: this test depends on MockRenderer::Sphere as well)
    Fixture() << Req("Declare") << "asdf" << "uniform float"
        << Req("Sphere") << 1.0f << -1.0f << 1.0f << 360.0f
            << "asdf" << std::vector<float>(1, 42.0f);
}

//--------------------------------------------------
RtVoid MockRenderer::DepthOfField(RtFloat fstop, RtFloat focallength,
                                  RtFloat focaldistance)
{
    CheckParams() << Req("DepthOfField");
    if(fstop == FLT_MAX)
    {
        BOOST_CHECK_EQUAL(focallength, FLT_MAX);
        BOOST_CHECK_EQUAL(focaldistance, FLT_MAX);
    }
    else
    {
        CheckParams() << fstop << focallength << focaldistance;
    }
}
BOOST_AUTO_TEST_CASE(DepthOfField_three_args_test)
{
    Fixture() << Req("DepthOfField") << 1.0f << 42.0f << 42.5f;
}
BOOST_AUTO_TEST_CASE(DepthOfField_no_args_test)
{
    Fixture() << Req("DepthOfField");
}


//--------------------------------------------------
RtVoid MockRenderer::ColorSamples(const FloatArray& nRGB,
                                  const FloatArray& RGBn)
{
    CheckParams() << Req("ColorSamples") << nRGB << RGBn;
}
BOOST_AUTO_TEST_CASE(ColorSamples_handler_test)
{
    Fixture() << Req("ColorSamples") << std::vector<float>(12, 1.0f)
        << std::vector<float>(12, 1.0f);
}


//--------------------------------------------------
RtVoid MockRenderer::LightSource(RtConstToken shadername,
                                        RtConstToken name,
                                        const ParamList& pList)
{
    CheckParams() << Req("LightSource") << shadername
        << StringOrInt(name) << pList;
}
BOOST_AUTO_TEST_CASE(LightSource_integer_id_test)
{
    // Test integer light identifiers
    Fixture() << Req("LightSource") << "blahlight" << 10;
}
BOOST_AUTO_TEST_CASE(LightSource_string_id_test)
{
    // Test string light identifiers
    Fixture() << Req("LightSource") << "blahlight" << "stringName";
}


//--------------------------------------------------
RtVoid MockRenderer::Illuminate(RtConstToken name, RtBoolean onoff)
{
    CheckParams() << Req("Illuminate") << StringOrInt(name)
        << static_cast<int>(onoff);
}
BOOST_AUTO_TEST_CASE(Illuminate_handler_test)
{
    Fixture()
        << Req("LightSource") << "blahlight" << 10
        << Req("LightSource") << "asdflight" << 11
        << Req("LightSource") << "qwerlight" << "handleName"
        << Req("Illuminate")  << 11 << 1
        << Req("Illuminate")  << 10 << 0
        << Req("Illuminate")  << "handleName" << 0;
}


//--------------------------------------------------
RtVoid MockRenderer::Basis(RtConstBasis ubasis, RtInt ustep,
                           RtConstBasis vbasis, RtInt vstep)
{
    BOOST_CHECK_EQUAL(g_mockBasis, ubasis);
    const float* vbStart = reinterpret_cast<const float*>(vbasis);
    CheckParams() << Req("Basis")
        << IgnoreParam << ustep <<
        std::vector<float>(vbStart, vbStart+16) << vstep;
}
BOOST_AUTO_TEST_CASE(Basis_handler_test)
{
    Fixture() << Req("Basis") << "b-spline"
        << 1 << std::vector<float>(16, 2.0f) << 42;

}


//--------------------------------------------------
RtVoid MockRenderer::SubdivisionMesh(RtConstToken scheme,
                                     const IntArray& nvertices,
                                     const IntArray& vertices,
                                     const TokenArray& tags,
                                     const IntArray& nargs,
                                     const IntArray& intargs,
                                     const FloatArray& floatargs,
                                     const ParamList& pList)
{
    CheckParams() << Req("SubdivisionMesh")
        << scheme << nvertices << vertices
        << tags << nargs << intargs << floatargs << pList;
}

BOOST_AUTO_TEST_CASE(SubdivisionMesh_test)
{
    std::vector<int> nvertices, vertices, nargs, intargs;
    std::vector<float> floatargs, P;
    std::vector<std::string> tags;

    nvertices += 4, 4;
    vertices += 0, 1, 4, 3,
                1, 2, 5, 4;
    tags += "interpolateboundary", "crease";
    nargs += 0, 0,  2, 1;
    intargs += 1, 4;
    floatargs += 2.5f;

    P += -1, -1, 0,
          0, -1, 0,
          1, -1, 0,
         -1,  1, 0,
          0,  1, 0,
          1,  1, 0;

    // Full form
    Fixture() << Req("SubdivisionMesh")
        << "catmull-clark" << nvertices << vertices << tags << nargs
        << intargs << floatargs << "P" << P;

    // Abbreviated form
    Fixture() << Req("SubdivisionMesh")
        << "catmull-clark" << nvertices << vertices
        << IgnoreParam << IgnoreParam << IgnoreParam << IgnoreParam
        << "P" << P;
}


//--------------------------------------------------
RtVoid MockRenderer::Hyperboloid(RtConstPoint point1, RtConstPoint point2,
                                 RtFloat thetamax, const ParamList& pList)
{
    CheckParams() << Req("Hyperboloid") << point1[0] << point1[1] << point1[2]
        << point2[0] << point2[1] << point2[2] << thetamax
        << pList;
}
BOOST_AUTO_TEST_CASE(Hyperboloid_handler_test)
{
    Fixture() << Req("Hyperboloid") << 1.0f << 2.0f << 3.0f
        << 4.0f << 5.0f << 6.0f << 360.0f;
}


//--------------------------------------------------
RtVoid MockRenderer::Procedural(RtPointer data, RtConstBound bound,
                                RtProcSubdivFunc refineproc,
                                RtProcFreeFunc freeproc)
{
    // The following checking is specific to the ProcRunProgram procedural.
    BOOST_CHECK_EQUAL(refineproc, mockProcSubdivFunc);

    // The following checking is valid for ProcRunProgram and ProcDynamicLoad
    const char** dataArray = reinterpret_cast<const char**>(data);
    const float* boundArray = reinterpret_cast<const float*>(bound);
    CheckParams() << Req("Procedural") << IgnoreParam
        << std::vector<std::string>(dataArray, dataArray+2)
        << std::vector<float>(boundArray, boundArray+6);
}
BOOST_AUTO_TEST_CASE(Procedural_handler_test)
{
    std::vector<std::string> args; args += "progname", "some arg string";
    Fixture() << Req("Procedural") << "RunProgram"
        << args << std::vector<float>(6,1.0f);
}


//--------------------------------------------------
RtVoid MockRenderer::ObjectBegin(RtConstToken name)
{
    CheckParams() << Req("ObjectBegin")
        << StringOrInt(name);
}
BOOST_AUTO_TEST_CASE(ObjectBegin_integer_id_test)
{
    Fixture() << Req("ObjectBegin") << 42;
}
BOOST_AUTO_TEST_CASE(ObjectBegin_string_id_test)
{
    Fixture() << Req("ObjectBegin") << "a_string_handle";
}
BOOST_AUTO_TEST_CASE(ObjectBegin_bad_float_id_test)
{
    BOOST_CHECK_THROW(
        Fixture() << Req("ObjectBegin") << 42.5f,
        MockRendererError
    );
}


//------------------------------------------------------------------------------
// Tests for some of the autogenerated handler implementations.  Full test
// coverage would be quite a burden (there's almost 100 interface functions!),
// so only a few are tested here.
//
// The choices try to cover all Rt parameter types, and all cases where the
// request handler has to do something special or unusual.

//--------------------------------------------------
RtVoid MockRenderer::Sphere(RtFloat radius, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList)
{
    CheckParams() << Req("Sphere") << radius << zmin << zmax << thetamax << pList;
}
BOOST_AUTO_TEST_CASE(Sphere_handler_test)
{
    // Test RiSphere; also test the handling of valid parameter lists.
    Fixture() << Req("Sphere") << 2.5f << -1.0f << 1.0f << 360.0f
        << "uniform float blah" << std::vector<float>(2, 42.25f)
        << "Cs" << std::vector<float>(4, 2.0f);
}


//--------------------------------------------------
RtVoid MockRenderer::Color(RtConstColor col)
{
    CheckParams() << Req("Color") << col[0] << col[1] << col[2];
}
BOOST_AUTO_TEST_CASE(Color_handler_test)
{
    Fixture() << Req("Color") << 1.0f << 2.0f << 3.0f;
}


//--------------------------------------------------
RtVoid MockRenderer::PixelFilter(RtFilterFunc function, RtFloat xwidth,
                                 RtFloat ywidth)
{
    BOOST_CHECK_EQUAL(function, mockFilterFunc);
    CheckParams() << Req("PixelFilter") << IgnoreParam << xwidth << ywidth;
}
BOOST_AUTO_TEST_CASE(PixelFilter_handler_test)
{
    Fixture() << Req("PixelFilter") << "sinc" << 7.0f << 7.5f;
}


//--------------------------------------------------
RtVoid MockRenderer::Transform(RtConstMatrix transform)
{
    const float* trans = reinterpret_cast<const float*>(transform);
    CheckParams() << Req("Transform")
        << std::vector<float>(trans, trans+16);
}
BOOST_AUTO_TEST_CASE(Transform_handler_test)
{
    float trans[16] = {
        1,2,3,4,
        5,6,7,8,
        9,-1,-2,-3,
        -4,-5,-6,-7
    };

    Fixture() << Req("Transform")
        << std::vector<float>(trans, trans+16);
}


//--------------------------------------------------
RtVoid MockRenderer::Points(const ParamList& pList)
{
    CheckParams() << Req("Points") << pList;
}
BOOST_AUTO_TEST_CASE(Points_handler_test)
{
    Fixture() << Req("Points")
        << "Cs" << std::vector<float>(12, 0.5f)
        << "P" << std::vector<float>(12, 1.5f);
}


//--------------------------------------------------
RtVoid MockRenderer::MotionBegin(const FloatArray& times)
{
    CheckParams() << Req("MotionBegin") << times;
}
BOOST_AUTO_TEST_CASE(MotionBegin_handler_test)
{
    std::vector<float> times;
    times += 0, 1, 2.5, 3, 4;
    Fixture() << Req("MotionBegin") << times;
}


//--------------------------------------------------
RtVoid MockRenderer::MakeOcclusion(const StringArray& picfiles,
                                   RtConstString shadowfile,
                                   const ParamList& pList)
{
    CheckParams() << Req("MakeOcclusion") << picfiles << shadowfile << pList;
}
BOOST_AUTO_TEST_CASE(MakeOcclusion_handler_test)
{
    std::vector<std::string> picFiles;
    picFiles += "pic1", "pic2", "pic3", "some_pic4", "asdf5", "Occlmap6.om";
    Fixture() << Req("MakeOcclusion") << picFiles << "ambient.map";
}


//------------------------------------------------------------------------------
// Test for parameter list handling, using RiOption as a proxy.
//
RtVoid MockRenderer::Option(RtConstToken name, const ParamList& pList)
{
    CheckParams() << Req("Option") << name << pList;
    // Check that the parameter list token count is correct.  Note that the
    // formula used below assumes there's only one Req in the token stream.
    BOOST_CHECK_EQUAL(2*pList.size(), g_fixture->tokens.size() - 2);
}


BOOST_AUTO_TEST_CASE(paramlist_int_array_value_test)
{
    Fixture() << Req("Option") << "some_option_name"
        << "uniform int asdf" << std::vector<int>(10, 42);
}
BOOST_AUTO_TEST_CASE(paramlist_float_array_value_test)
{
    Fixture() << Req("Option") << "some_option_name"
        << "uniform float asdf" << std::vector<float>(10, 2.5f);
}
BOOST_AUTO_TEST_CASE(paramlist_string_array_value_test)
{
    Fixture() << Req("Option") << "some_option_name"
        << "uniform string asdf" << std::vector<std::string>(10, "asdf_value");
}
BOOST_AUTO_TEST_CASE(extended_paramlist_test)
{
    // Check that many mixed params work fine.
    Fixture() << Req("Option") << "some_option_name"
        << "uniform int user_i" << std::vector<int>(1, 42)
        << "uniform float user_f" << std::vector<float>(1, 2.5f)
        << "uniform vector user_v" << std::vector<float>(3, 3.5f)
        << "uniform matrix user_m" << std::vector<float>(16, 4.5f)
        << "uniform string s1" << std::vector<std::string>(1, "blah")
        << "uniform string s2" << std::vector<std::string>(1, "s2val")
        << "uniform string s3" << std::vector<std::string>(1, "s3val")
        << "uniform string s4" << std::vector<std::string>(1, "s4val");
}

BOOST_AUTO_TEST_CASE(invalid_paramlist_unknown_primvar)
{
    // Check that unknown primvar names throw.
    BOOST_CHECK_THROW(
        Fixture() << Req("Option") << "some_option_name"
            << "asdf" << std::vector<float>(1, 42.25f),
        MockRendererError
    );
}
BOOST_AUTO_TEST_CASE(invalid_paramlist_invalid_primvar)
{
    // Check that invalid primvar strings throw.
    BOOST_CHECK_THROW(
        Fixture() << Req("Option") << "some_option_name"
            << "] bad_token" << std::vector<float>(1, 42.25f),
        MockRendererError
    );
}
BOOST_AUTO_TEST_CASE(invalid_paramlist_missing_token)
{
    // check that a missing token throws.
    BOOST_CHECK_THROW(
        Fixture() << Req("Option") << "some_option_name"
            << "P" << std::vector<float>(4, 42.25f)
            << std::vector<int>(1, 42),
        MockRendererError
    );
}

BOOST_AUTO_TEST_SUITE_END()

// vi: set et:
