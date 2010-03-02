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

#include <aqsis/aqsis.h>

#ifndef    AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

#include "ribsema.h"

#include <sstream>

#include <boost/test/auto_unit_test.hpp>

#include <boost/assign/std/vector.hpp>

#include <aqsis/math/math.h>
#include <aqsis/ri/ri.h>
#include <aqsis/ribparser.h>

using namespace boost::assign; // necessary for container initialisation operators.

using namespace Aqsis;


//==============================================================================
// mock objects, test fixtures and other utility code for the tests
//==============================================================================
//
// Unfortunately, writing tests for RibSema easily and elegantly requires a
// *lot* of setup.  At the centre of it all is a mock object MockParser which
// implements the IqRibParser interface and acts as the callback object for
// RibSema object during testing.
//
// The machinery below exists to help the insertion of valid token sequences
// into MockParser and to check those sequences against the output which
// appears as parameters to the appropriate RI function call.
//

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
    // We put these operator<<() in a namespace so that they can be introduced
    // into namespace boost in a controllable way.

    // Insert a std::vector into a stream in RIB format.
    template<typename T>
    std::ostream& operator<<(std::ostream& out, const std::vector<T>& v)
    {
        out << "[";
        for(int i = 0, end = v.size(); i < end; ++i)
        {
            out << v[i];
            if(i != end-1)
                out << " ";
        }
        out << "]";
        return out;
    }

    // Insert a Req into a stream.
    std::ostream& operator<<(std::ostream& out, const Req& r)
    {
        out << r.name;
        return out;
    }
}

// Introduce the printer funcs into both the global and boost namespaces so
// that lookup can find them.
namespace boost {
    using printer_funcs::operator<<;
}
using printer_funcs::operator<<;


//------------------------------------------------------------------------------

// Types for the current element of MockParser.
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
struct IgnoreParam {};

/** Cheap and nasty variant holding tokens for MockParser
 *
 * Yeah, boost::variant would allow me to avoid implementing this class, but
 * the error messages are absolutely insane, to the point of making the code
 * unmaintainable.
 */
class MockRibToken
{
    private:
        MockTokType m_type;

        int m_int;
        float m_float;
        std::string m_string;
        Req m_request;
        IqRibParser::TqIntArray m_ints;
        IqRibParser::TqFloatArray m_floats;
        IqRibParser::TqStringArray m_strings;

    public:
        MockRibToken(int i)
            : m_type(Type_Int), m_int(i) { }
        MockRibToken(float f)
            : m_type(Type_Float), m_float(f) { }
        MockRibToken(const char* s)
            : m_type(Type_String), m_string(s) { }
        MockRibToken(const std::string& s)
            : m_type(Type_String), m_string(s) { }
        MockRibToken(const Req& request)
            : m_type(Type_Request), m_request(request) { }
        MockRibToken(const IqRibParser::TqIntArray& v)
            : m_type(Type_IntArray), m_ints(v) { }
        MockRibToken(const IqRibParser::TqFloatArray& v)
            : m_type(Type_FloatArray), m_floats(v) { }
        MockRibToken(const IqRibParser::TqStringArray& v)
            : m_type(Type_StringArray), m_strings(v) { }
        MockRibToken(const IgnoreParam&)
            : m_type(Type_Ignore) { }

        MockTokType type() const { return m_type; }

        int getInt() const
                { BOOST_REQUIRE(m_type == Type_Int); return m_int; }
        float getFloat() const
                { BOOST_REQUIRE(m_type == Type_Float); return m_float; }
        const std::string& getString() const
                { BOOST_REQUIRE(m_type == Type_String); return m_string; }
        const Req& getReq() const
                { BOOST_REQUIRE(m_type == Type_Request); return m_request; }
        const IqRibParser::TqIntArray& getIntArray() const
                { BOOST_REQUIRE(m_type == Type_IntArray); return m_ints; }
        const IqRibParser::TqFloatArray& getFloatArray() const
                { BOOST_REQUIRE(m_type == Type_FloatArray); return m_floats; }
        const IqRibParser::TqStringArray& getStringArray() const
                { BOOST_REQUIRE(m_type == Type_StringArray); return m_strings; }

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
                case Type_Int:            return getInt() == rhs.getInt();
                case Type_Float:        return getFloat() == rhs.getFloat();
                case Type_String:        return getString() == rhs.getString();
                case Type_IntArray:        return getIntArray() == rhs.getIntArray();
                case Type_FloatArray:    return getFloatArray() == rhs.getFloatArray();
                case Type_StringArray:    return getStringArray() == rhs.getStringArray();
                case Type_Request:        return getReq() == rhs.getReq();
                case Type_Ignore:        return true;
            }
            BOOST_REQUIRE(0 && "unrecognised type??");
            return false;
        }

};

std::ostream& operator<<(std::ostream& out, const MockRibToken& tok)
{
    switch(tok.type())
    {
        case Type_Int:             out << tok.getInt();        break;
        case Type_Float:         out << tok.getFloat();        break;
        case Type_String:         out << tok.getString();        break;
        case Type_IntArray:     out << tok.getIntArray();    break;
        case Type_FloatArray:     out << tok.getFloatArray();    break;
        case Type_StringArray:     out << tok.getStringArray();break;
        case Type_Request:         out << tok.getReq();        break;
        case Type_Ignore:        out << "IGNORE";            break;
    }
    return out;
}

// Compare the contents of a std::vector to a raw C array containing the same
// type.
template<typename T>
bool arrayEqual(const std::vector<T>& v1, const void* v2)
{
    const T* v2T = reinterpret_cast<const T*>(v2);
    for(int i = 0, end = v1.size(); i < end; ++i)
    {
        if(v1[i] != v2T[i])
            return false;
    }
    return true;
}

// Compare a token containing an array to a raw C array.
//
// assert if the token doesn't contain an array.
bool tokenVoidCompareEqual(const MockRibToken& lhs, const void* rhs)
{
    switch(lhs.type())
    {
        case Type_IntArray:        return arrayEqual(lhs.getIntArray(), rhs);
        case Type_FloatArray:    return arrayEqual(lhs.getFloatArray(), rhs);
        case Type_StringArray:    return arrayEqual(lhs.getStringArray(), rhs);
        default:
            BOOST_REQUIRE(0 && "MockRibToken void* compare not implemented for type");
    }
    return false;
}


//------------------------------------------------------------------------------

typedef std::vector<MockRibToken> TokenVec;

// Mock implementation of the RIB parser interface for testing purposes.
//
// This "parser" uses a vector of boost::any to describe the input which
// should be provided to the request handler object.  When data is requested,
// the mock parser just uses boost::any_cast to cast the current input element
// to the requested type; an exception is thrown if the types don't match.
//
// The input data should be provided via the params() accessor before an
// attempt is made to use the parser.
class MockParser : public IqRibParser
{
    private:
        TokenVec m_params;
        RibSema& m_handler;
        int m_tokenPos;
        int m_currentRequestStart;
        std::vector<TqFloatArray> m_floatVecStorage;

        void discardIgnoreToks()
        {
            // discard any empty tokens.
            while(m_tokenPos < static_cast<int>(m_params.size())
                    && m_params[m_tokenPos].type() == Type_Ignore)
            {
                ++m_tokenPos;
            }
        }
        void checkNextType(MockTokType nextTokType)
        {
            discardIgnoreToks();
            if(m_params[m_tokenPos].type() != nextTokType)
                AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
                        "missmatched token type " << nextTokType << " requested");
        }
    public:
        // Helper class to insert parameters into the parser.
        MockParser(RibSema& handler)
            : m_params(),
            m_handler(handler),
            m_tokenPos(0),
            m_currentRequestStart(0),
            m_floatVecStorage()
        { }

        TokenVec& params()
        {
            return m_params;
        }

        TokenVec::const_iterator currParams() const
        {
            return m_params.begin() + m_currentRequestStart;
        }

        int currentRequestStart() const
        {
            return m_currentRequestStart;
        }

        virtual bool parseNextRequest()
        {
            m_currentRequestStart = m_tokenPos;
            m_handler.handleRequest(m_params[m_tokenPos++].getReq().name, *this );
            return false;
        }

        virtual void pushInput(std::istream& inStream, const std::string& name,
                const TqCommentCallback& callback) {}
        virtual void popInput() {}
        virtual SqRibPos streamPos()
        {
            return SqRibPos(0, m_tokenPos, "test_token_stream");
        }

        virtual int getInt()
        {
            checkNextType(Type_Int);
            return m_params[m_tokenPos++].getInt();
        }
        virtual float getFloat()
        {
            checkNextType(Type_Float);
            return m_params[m_tokenPos++].getFloat();
        }
        virtual std::string getString()
        {
            checkNextType(Type_String);
            return m_params[m_tokenPos++].getString();
        }

        virtual const TqIntArray& getIntArray()
        {
            checkNextType(Type_IntArray);
            return m_params[m_tokenPos++].getIntArray();
        }
        virtual const TqFloatArray& getFloatArray(int length = -1)
        {
            discardIgnoreToks();
            if(length >= 0 && m_params[m_tokenPos].type() == Type_Float)
            {
                m_floatVecStorage.push_back(TqFloatArray(length,0));
                // read in individual floats for the array
                TqFloatArray& array = m_floatVecStorage.back();
                for(int i = 0; i < length; ++i)
                    array[i] = m_params[m_tokenPos++].getFloat();
                return array;
            }
            else
            {
                checkNextType(Type_FloatArray);
                const TqFloatArray& array = m_params[m_tokenPos++].getFloatArray();
                if(length >= 0 && static_cast<int>(array.size()) != length)
                    throw std::runtime_error("Bad array length detected.");
                return array;
            }
        }
        virtual const TqStringArray& getStringArray()
        {
            checkNextType(Type_StringArray);
            return m_params[m_tokenPos++].getStringArray();
        }

        virtual void getParamList(IqRibParamListHandler& paramHandler)
        {
            discardIgnoreToks();
            while(m_tokenPos < static_cast<int>(m_params.size())
                    && m_params[m_tokenPos].type() != Type_Request)
            {
                checkNextType(Type_String);
                paramHandler.readParameter(getString(), *this);
            }
        }

        virtual EqRibToken peekNextType()
        {
            if(m_tokenPos >= static_cast<int>(m_params.size()))
                return Tok_RequestEnd;
            discardIgnoreToks();
            switch(m_params[m_tokenPos].type())
            {
                case Type_Int:            return Tok_Int;
                case Type_Float:        return Tok_Float;
                case Type_String:        return Tok_String;
                case Type_IntArray:
                case Type_FloatArray:
                case Type_StringArray:    return Tok_Array;
                case Type_Request:        return Tok_RequestEnd;
                case Type_Ignore:        break;
            }
            BOOST_REQUIRE(0 && "peekNextType - type unknown");
            return Tok_Int;
        }

        virtual const TqIntArray& getIntParam()
        {
            return getIntArray();
        }
        virtual const TqFloatArray& getFloatParam()
        {
            return getFloatArray();
        }
        virtual const TqStringArray& getStringParam()
        {
            return getStringArray();
        }
};

class MockRenderer : public Ri::Renderer
{
    public:
        MockRenderer() {}

        // Define all methods, except those which are tested.
        /*[[[cog
        import cog
        import sys, os
        sys.path.append(os.getcwd())
        from cogutils import *

        riXml = parseXmlTree('ri.xml')

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
            'ObjectInstance',
            'MotionBegin',
            'MakeOcclusion',
            'ErrorHandler',
        ])

        for p in riXml.findall('Procedures/Procedure'):
            if p.haschild('Rib'):
                decl = riCxxMethodDecl(p)
                procName = p.findtext('Name')
                if procName in testedMethods:
                    cog.outl('// %s is tested' % (procName,))
                    decl = 'virtual %s;' % (decl,)
                else:
                    body = ''
                    if p.findtext('ReturnType') != 'RtVoid':
                        body = 'return 0;'
                    decl = 'virtual %s {%s}' % (decl,body)
                cog.outl(wrapDecl(decl, 72, wrapIndent=20))
        ]]]*/
        // Declare is tested
        virtual RtToken Declare(RtConstString name, RtConstString declaration);
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
        virtual RtLightHandle LightSource(RtConstToken name,
                            const ParamList& pList);
        virtual RtLightHandle AreaLightSource(RtConstToken name,
                            const ParamList& pList) {return 0;}
        // Illuminate is tested
        virtual RtVoid Illuminate(RtLightHandle light, RtBoolean onoff);
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
                            const FloatArray& flt, const TokenArray& str,
                            const ParamList& pList) {}
        // Procedural is tested
        virtual RtVoid Procedural(RtPointer data, RtConstBound bound,
                            RtProcSubdivFunc refineproc,
                            RtProcFreeFunc freeproc);
        virtual RtVoid Geometry(RtConstToken type, const ParamList& pList) {}
        virtual RtVoid SolidBegin(RtConstToken type) {}
        virtual RtVoid SolidEnd() {}
        // ObjectBegin is tested
        virtual RtObjectHandle ObjectBegin();
        virtual RtVoid ObjectEnd() {}
        // ObjectInstance is tested
        virtual RtVoid ObjectInstance(RtObjectHandle handle);
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
        // ErrorHandler is tested
        virtual RtVoid ErrorHandler(RtErrorFunc handler);
        virtual RtVoid ReadArchive(RtConstToken name,
                            RtArchiveCallback callback,
                            const ParamList& pList) {}
        //[[[end]]]

        virtual RtVoid ArchiveRecord(RtConstToken type, const char* string) {}
        virtual RtPoint* TransformPoints(RtConstToken fromspace, RtConstToken tospace,
                            RtInt npoints, RtPoint points[]) const {return 0;}

        virtual RtFilterFunc     GetFilterFunc(RtConstToken name) const {return 0;}
        virtual RtConstBasis*    GetBasis(RtConstToken name) const {return 0;}
        virtual RtErrorFunc      GetErrorFunc(RtConstToken name) const {return 0;}
        virtual RtProcSubdivFunc GetProcSubdivFunc(RtConstToken name) const {return 0;}
        virtual RtProcFreeFunc   GetProcFreeFunc() const {return 0;}
};


/** Manage mappings from object or light handles to pointers for testing.
 *
 * If we want to check that the mapping is happening correctly we need to
 * record both the RIB object id and the returned pointer.
 */
class HandleManager
{
    private:
        int m_currHandle;
        typedef std::vector<std::pair<MockRibToken, RtPointer> > TqHandleMap;
        TqHandleMap m_handleMap;

    public:
        HandleManager()
            : m_currHandle(0),
            m_handleMap()
        { }

        RtPointer insertHandle(const MockRibToken& id)
        {
            BOOST_REQUIRE(id.type() == Type_Int || id.type() == Type_String);
            RtPointer p = reinterpret_cast<RtPointer>(++m_currHandle);
            m_handleMap.push_back(std::make_pair(id, p));
            return p;
        }
        const MockRibToken& lookup(RtPointer handle) const
        {
            for(TqHandleMap::const_iterator i = m_handleMap.begin();
                    i < m_handleMap.end(); ++i)
            {
                if(handle == i->second)
                    return i->first;
            }
            throw std::runtime_error("sequence id not found");
            return m_handleMap[0].first;
        }
};


// Fixture used in all tests, containing an instance of RibSema to
// be tested, along with instances of supporting classes.
//
// The constructor installs the instance of RequestHandlerFixture into the
// global variable g_fixture, so only one of these should exist at any time.
struct RequestHandlerFixture
{
    // Mock renderer
    MockRenderer renderer;
    // Object to be tested
    RibSema handler;
    // Mock parser
    MockParser parser;
    // Flag indicating that a request has been checked.
    bool hasBeenChecked;
    // Manager for holding handles
    HandleManager handleManager;

    RequestHandlerFixture();
    ~RequestHandlerFixture();
};

// global to enable Ri* function implementations to validate the function
// parameters against those which were input to the handler via the parser.
static RequestHandlerFixture* g_fixture = 0;

RequestHandlerFixture::RequestHandlerFixture()
    : renderer(),
    handler(renderer),
    parser(handler),
    hasBeenChecked(false)
{
    // Make sure that only one instance can exist at a time.
    BOOST_REQUIRE(!g_fixture);
    g_fixture = this;
}

RequestHandlerFixture::~RequestHandlerFixture()
{
    g_fixture = 0;
}


// Inserter for inserting a token stream into MockParser
class Insert
{
    private:
        MockParser& m_parser;
    public:
        Insert(MockParser& parser) : m_parser(parser) {}
        ~Insert()
        {
            m_parser.parseNextRequest();
            BOOST_REQUIRE(g_fixture);
            BOOST_CHECK(g_fixture->hasBeenChecked);
            g_fixture->hasBeenChecked = false;
        }

        Insert& operator<<(const MockRibToken& tok)
        {
            m_parser.params().push_back(tok);
            return *this;
        }
};


// struct used to hold a param list for insertion into a CheckParams() sequence.
struct ParamList
{
    int count;
    RtToken* tokens;
    RtPointer* values;
    ParamList(int count, RtToken tokens[], RtPointer values[])
        : count(count), tokens(tokens), values(values)
    {}
};

// Checker class for checking parameters passed to an RI function against the
// initial values provided to MockParser.
class CheckParams
{
    private:
        TokenVec::const_iterator m_currParam;

    public:
        CheckParams() : m_currParam()
        {
            BOOST_REQUIRE(g_fixture);
            m_currParam = g_fixture->parser.currParams();
        }

        ~CheckParams()
        {
            g_fixture->hasBeenChecked = true;
        }

        CheckParams& operator<<(const MockRibToken& val)
        {
            BOOST_CHECK_EQUAL(*(m_currParam++), val);
            return *this;
        }

        CheckParams& operator<<(const IgnoreParam&)
        {
            ++m_currParam;
            return *this;
        }

        CheckParams& operator<<(const ParamList& pList)
        {
            for(int i = 0; i < pList.count; ++i)
            {
                BOOST_CHECK_EQUAL((m_currParam++)->getString(), pList.tokens[i]);
                BOOST_CHECK_PREDICATE(tokenVoidCompareEqual,
                        (*(m_currParam++)) (pList.values[i]) );
            }
            return *this;
        }
};


//==============================================================================
// The actual test cases follow below.
//==============================================================================
//
// Each test is written in two parts
//
// 1) The tester function itself inserts request parameters into the parser
// and kicks off the parsing of the request.  This is done as follows:
//
//   RequestHandlerFixture f;
//   Insert(f.parser) << Req("SomeRequestName") << 1.0 << 1.0 << "asdf";
//
// f contains the mock parser and an instance of RibSema which is
// the component that we're actually testing.  The Insert() creates a temporary
// object of type Insert() which allows insertion of request parameters into
// the parser.  The destructor of the temporary Insert instance is called at
// the end of the statement which kicks off parsing of the inserted parameters.
//
//
// 2) The RI function which is going to get called by the request handler.
// This should check that the parameters were correctly passed through the
// interface using an instance of CheckParams() as follows:
//
// RiSomeRequestName(RtInt var1, RtInt var2, RtString var3)
// {
//     CheckParams() << Req("SomeRequestName") << var1 << var2 << var3;
// }
//
// CheckParams uses a global reference to the most recently created
// RequestHandlerFixture, so can access the parameters passed by Insert() and
// check that these are the same.
//
// For some RI functions the RIB form and RI form are sufficiently different
// that some parameters have to be checked "by hand" without ChechParams() -
// these parameters should be explicitly ignored in the CheckParams() insertion
// stream using the IgnoreParam() manipulator.
//
// Finally, a renderman parameter list can be passed in using the ParamList
// object.
//

//------------------------------------------------------------------------------
// Tests for handler functions with hand-written implementations.

BOOST_AUTO_TEST_CASE(RIB_version_test)
{
    // Test the RIB-only version token... this doesn't actually do anything
    // right now but shouldn't generate an error
    RequestHandlerFixture f;
    f.parser.params() += Req("version"), 3.03f;
    f.parser.parseNextRequest();
}


RtToken MockRenderer::Declare(RtConstString name, RtConstString declaration) {return 0;}
RtVoid MockRenderer::DepthOfField(RtFloat fstop, RtFloat focallength,
                                  RtFloat focaldistance) {}
RtVoid MockRenderer::PixelFilter(RtFilterFunc function, RtFloat xwidth,
                                 RtFloat ywidth) {}
RtVoid MockRenderer::ColorSamples(const FloatArray& nRGB,
                                  const FloatArray& RGBn) {}
RtVoid MockRenderer::Option(RtConstToken name, const ParamList& pList) {}
RtVoid MockRenderer::Color(RtConstColor Cq) {}
RtLightHandle MockRenderer::LightSource(RtConstToken name,
                                        const ParamList& pList) {return 0;}
RtVoid MockRenderer::Illuminate(RtLightHandle light, RtBoolean onoff) {}
RtVoid MockRenderer::Transform(RtConstMatrix transform) {}
RtVoid MockRenderer::Basis(RtConstBasis ubasis, RtInt ustep,
                           RtConstBasis vbasis, RtInt vstep) {}
RtVoid MockRenderer::SubdivisionMesh(RtConstToken scheme,
                                     const IntArray& nvertices,
                                     const IntArray& vertices,
                                     const TokenArray& tags,
                                     const IntArray& nargs,
                                     const IntArray& intargs,
                                     const FloatArray& floatargs,
                                     const ParamList& pList) {}
RtVoid MockRenderer::Sphere(RtFloat radius, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList) {}
RtVoid MockRenderer::Hyperboloid(RtConstPoint point1, RtConstPoint point2,
                                 RtFloat thetamax, const ParamList& pList) {}
RtVoid MockRenderer::Points(const ParamList& pList) {}
RtVoid MockRenderer::Procedural(RtPointer data, RtConstBound bound,
                                RtProcSubdivFunc refineproc,
                                RtProcFreeFunc freeproc) {}
RtObjectHandle MockRenderer::ObjectBegin() {return 0;}
RtVoid MockRenderer::ObjectInstance(RtObjectHandle handle) {}
RtVoid MockRenderer::MotionBegin(const FloatArray& times) {}
RtVoid MockRenderer::MakeOcclusion(const StringArray& picfiles,
                                   RtConstString shadowfile,
                                   const ParamList& pList) {}
RtVoid MockRenderer::ErrorHandler(RtErrorFunc handler) {}


#if 0

//--------------------------------------------------
RtToken MockRenderer::Declare(RtConstString name, RtConstString declaration)
{
    CheckParams() << Req("Declare") << name << declaration;
    g_fixture->hasBeenChecked = true;
    return 0;
}
BOOST_AUTO_TEST_CASE(Declare_handler_test)
{
    RequestHandlerFixture f;
    Insert(f.parser) << Req("Declare") << "asdf" << "uniform float";
    Insert(f.parser) << Req("Sphere") << 1.0f << -1.0f << 1.0f << 360.0f
                        << "asdf" << IqRibParser::TqFloatArray(1, 42.0f);
}


//--------------------------------------------------
RtVoid MockRenderer::DepthOfField(RtFloat fstop, RtFloat focallength, RtFloat focaldistance)
{
    if(fstop == FLT_MAX)
    {
        BOOST_CHECK_EQUAL(g_fixture->parser.params().size(), 1U);
        BOOST_CHECK_EQUAL(focallength, FLT_MAX);
        BOOST_CHECK_EQUAL(focaldistance, FLT_MAX);
        g_fixture->hasBeenChecked = true;
    }
    else
    {
        CheckParams() << Req("DepthOfField") << fstop << focallength << focaldistance;
    }
}
BOOST_AUTO_TEST_CASE(DepthOfField_three_args_test)
{
    RequestHandlerFixture f;
    Insert(f.parser) << Req("DepthOfField") << 1.0f << 42.0f << 42.5f;
}
BOOST_AUTO_TEST_CASE(DepthOfField_no_args_test)
{
    RequestHandlerFixture f;
    Insert(f.parser) << Req("DepthOfField");
}


//--------------------------------------------------
RtVoid MockRenderer::ColorSamples(RtInt N, RtFloat nRGB[], RtFloat RGBn[])
{
    CheckParams() << Req("ColorSamples")
        << IqRibParser::TqFloatArray(nRGB, nRGB + 3*N)
        << IqRibParser::TqFloatArray(RGBn, RGBn + 3*N);
}
BOOST_AUTO_TEST_CASE(ColorSamples_handler_test)
{
    RequestHandlerFixture f;
    Insert(f.parser) << Req("ColorSamples") << IqRibParser::TqFloatArray(12, 1.0f)
        << IqRibParser::TqFloatArray(12, 1.0f);
}


//--------------------------------------------------
RtLightHandle MockRenderer::LightSource(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    CheckParams() << Req("LightSource") << name << IgnoreParam()
        << ParamList(count, tokens, values);
    return g_fixture->handleManager.insertHandle(g_fixture->parser.currParams()[2]);
}
BOOST_AUTO_TEST_CASE(LightSource_integer_id_test)
{
    // Test integer light identifiers
    RequestHandlerFixture f;
    Insert(f.parser) << Req("LightSource") << "blahlight" << 10;
}
BOOST_AUTO_TEST_CASE(LightSource_string_id_test)
{
    // Test string light identifiers
    RequestHandlerFixture f;
    Insert(f.parser) << Req("LightSource") << "blahlight" << "stringName";
}


//--------------------------------------------------
RtVoid MockRenderer::Illuminate(RtLightHandle light, RtBoolean onoff)
{
    CheckParams()
        << Req("Illuminate")
        << g_fixture->handleManager.lookup(light)
        << static_cast<int>(onoff);
}
BOOST_AUTO_TEST_CASE(Illuminate_handler_test)
{
    RequestHandlerFixture f;
    Insert(f.parser) << Req("LightSource") << "blahlight" << 10;
    Insert(f.parser) << Req("LightSource") << "asdflight" << 11;
    Insert(f.parser) << Req("LightSource") << "qwerlight" << "handleName";
    Insert(f.parser) << Req("Illuminate")  << 11 << 1;
    Insert(f.parser) << Req("Illuminate")  << 10 << 0;
    Insert(f.parser) << Req("Illuminate")  << "handleName" << 0;
}
BOOST_AUTO_TEST_CASE(Illuminate_bad_int_handle_test)
{
    RequestHandlerFixture f;
    BOOST_CHECK_THROW(
        Insert(f.parser) << Req("Illuminate") << 10 << 1,
        XqParseError
    );
}
BOOST_AUTO_TEST_CASE(Illuminate_bad_string_handle_test)
{
    RequestHandlerFixture f;
    BOOST_CHECK_THROW(
        Insert(f.parser) << Req("Illuminate") << "asdf" << 1,
        XqParseError
    );
}


//--------------------------------------------------
RtVoid MockRenderer::Basis(RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep)
{
    BOOST_CHECK_EQUAL(::RiBSplineBasis, ubasis);
    float* vbStart = reinterpret_cast<float*>(vbasis);
    CheckParams() << Req("Basis")
        << IgnoreParam() << ustep <<
        IqRibParser::TqFloatArray(vbStart, vbStart+16) << vstep;
}
BOOST_AUTO_TEST_CASE(Basis_handler_test)
{
    RequestHandlerFixture f;
    Insert(f.parser) << Req("Basis") << "b-spline"
        << 1 << IqRibParser::TqFloatArray(16, 2.0f) << 42;
}


//--------------------------------------------------
RtVoid MockRenderer::SubdivisionMesh(RtToken scheme, RtInt nfaces, RtInt nvertices[],
        RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[],
        RtInt intargs[], RtFloat floatargs[],
        RtInt count, RtToken tokens[], RtPointer values[])
{
    // total number of distinct vertices (number in "P" array)
    int totVertices = 0;
    // total number of face vertices (size of vertices[])
    int vertNum = 0;
    for(int face = 0; face < nfaces; ++face)
    {
        for(int i = 0; i < nvertices[face]; ++i)
        {
            totVertices = max(vertices[vertNum], totVertices);
            ++vertNum;
        }
    }
    int numIntArgs = 0;
    int numFloatArgs = 0;
    for(int tag = 0; tag < ntags; ++tag)
    {
        numIntArgs += nargs[2*tag];
        numFloatArgs += nargs[2*tag + 1];
    }
    CheckParams() << Req("SubdivisionMesh") << scheme
        << IqRibParser::TqIntArray(nvertices, nvertices + nfaces)
        << IqRibParser::TqIntArray(vertices, vertices + vertNum)
        << IqRibParser::TqStringArray(tags, tags + ntags)
        << IqRibParser::TqIntArray(nargs, nargs + 2*ntags)
        << IqRibParser::TqIntArray(intargs, intargs + numIntArgs)
        << IqRibParser::TqFloatArray(floatargs, floatargs + numFloatArgs)
        << ParamList(count, tokens, values);
}

BOOST_AUTO_TEST_CASE(SubdivisionMesh_full_form_test)
{
    IqRibParser::TqIntArray nvertices, vertices, nargs, intargs;
    IqRibParser::TqFloatArray floatargs, P;
    IqRibParser::TqStringArray tags;

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

    RequestHandlerFixture f;
    Insert(f.parser) << Req("SubdivisionMesh")
        << "catmull-clark" << nvertices << vertices << tags << nargs
        << intargs << floatargs << "P" << P;
}

BOOST_AUTO_TEST_CASE(SubdivisionMesh_abbreviated_form_test)
{
    IqRibParser::TqIntArray nvertices, vertices, nargs, intargs;
    IqRibParser::TqFloatArray floatargs, P;
    IqRibParser::TqStringArray tags;

    nvertices += 4, 4;
    vertices += 0, 1, 4, 3,
                1, 2, 5, 4;

    P += -1, -1, 0,
          0, -1, 0,
          1, -1, 0,
         -1,  1, 0,
          0,  1, 0,
          1,  1, 0;

    RequestHandlerFixture f;
    Insert(f.parser) << Req("SubdivisionMesh")
        << "catmull-clark" << nvertices << vertices
        << IgnoreParam() << IgnoreParam() << IgnoreParam() << IgnoreParam()
        << "P" << P;
}


//--------------------------------------------------
RtVoid MockRenderer::Hyperboloid(RtPoint point1, RtPoint point2, RtFloat thetamax,
        RtInt count, RtToken tokens[], RtPointer values[])
{
    CheckParams() << Req("Hyperboloid") << point1[0] << point1[1] << point1[2]
        << point2[0] << point2[1] << point2[2] << thetamax
        << ParamList(count, tokens, values);
}
BOOST_AUTO_TEST_CASE(HyperboloidV_handler_test)
{
    RequestHandlerFixture f;
    Insert(f.parser) << Req("Hyperboloid") << 1.0f << 2.0f << 3.0f
        << 4.0f << 5.0f << 6.0f << 360.0f;
}


//--------------------------------------------------
RtVoid MockRenderer::Procedural(RtPointer data, RtBound bound, RtProcSubdivFunc refineproc, RtProcFreeFunc freeproc)
{
    // All the standard procedurals should have RiProcFree
    BOOST_CHECK_EQUAL(freeproc, RiProcFree);

    // The following checking is specific to the ProcRunProgram procedural.
    BOOST_CHECK_EQUAL(refineproc, RiProcRunProgram);

    // The following checking is valid for ProcRunProgram and ProcDynamicLoad
    char** dataArray = reinterpret_cast<char**>(data);
    float* boundArray = reinterpret_cast<float*>(bound);
    CheckParams() << Req("Procedural") << IgnoreParam()
        << IqRibParser::TqStringArray(dataArray, dataArray+2)
        << IqRibParser::TqFloatArray(boundArray, boundArray+6);
}
BOOST_AUTO_TEST_CASE(Procedural_handler_test)
{
    RequestHandlerFixture f;
    IqRibParser::TqStringArray args; args += "progname", "some arg string";
    Insert(f.parser) << Req("Procedural") << "RunProgram"
        << args << IqRibParser::TqFloatArray(6,1.0f);
}
BOOST_AUTO_TEST_CASE(Procedural_unknown_procedural_test)
{
    RequestHandlerFixture f;
    BOOST_CHECK_THROW(
        Insert(f.parser) << Req("Procedural") << "SomeNonexistantProcName"
            << IqRibParser::TqStringArray(1,"asdf") << IqRibParser::TqFloatArray(6,1.0f),
        XqParseError
    );
}


//--------------------------------------------------
RtObjectHandle MockRenderer::ObjectBegin()
{
    CheckParams() << Req("ObjectBegin");
    return g_fixture->handleManager.insertHandle(g_fixture->parser.currParams()[1]);
}
BOOST_AUTO_TEST_CASE(ObjectBegin_integer_id_test)
{
    RequestHandlerFixture f;
    Insert(f.parser) << Req("ObjectBegin") << 42;
}
BOOST_AUTO_TEST_CASE(ObjectBegin_string_id_test)
{
    RequestHandlerFixture f;
    Insert(f.parser) << Req("ObjectBegin") << "a_string_handle";
}
BOOST_AUTO_TEST_CASE(ObjectBegin_bad_float_id_test)
{
    RequestHandlerFixture f;
    BOOST_CHECK_THROW(
        Insert(f.parser) << Req("ObjectBegin") << 42.5f,
        XqParseError
    );
}


//--------------------------------------------------
RtVoid MockRenderer::ObjectInstance(RtObjectHandle handle)
{
    CheckParams() << Req("ObjectInstance")
        << g_fixture->handleManager.lookup(handle);
}
BOOST_AUTO_TEST_CASE(ObjectInstance_integer_id_lookup)
{
    RequestHandlerFixture f;
    Insert(f.parser) << Req("ObjectBegin") << 42;
    Insert(f.parser) << Req("ObjectBegin") << 2;
    Insert(f.parser) << Req("ObjectInstance") << 2;
    Insert(f.parser) << Req("ObjectInstance") << 42;
}
BOOST_AUTO_TEST_CASE(ObjectInstance_string_id_lookup)
{
    RequestHandlerFixture f;
    Insert(f.parser) << Req("ObjectBegin") << "a_string_handle";
    Insert(f.parser) << Req("ObjectInstance") << "a_string_handle";
}
BOOST_AUTO_TEST_CASE(ObjectInstance_undeclared_error_test)
{
    RequestHandlerFixture f;
    BOOST_CHECK_THROW(
        Insert(f.parser) << Req("ObjectInstance") << 1,
        XqParseError
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
RtVoid MockRenderer::Sphere(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax,
        RtInt count, RtToken tokens[], RtPointer values[])
{
    CheckParams() << Req("Sphere") << radius << zmin << zmax << thetamax
        << ParamList(count, tokens, values);
}
BOOST_AUTO_TEST_CASE(SphereV_handler_test)
{
    // Test RiSphere; also test the handling of valid parameter lists.
    RequestHandlerFixture f;
    Insert(f.parser) << Req("Sphere") << 2.5f << -1.0f << 1.0f << 360.0f
        << "uniform float blah" << IqRibParser::TqFloatArray(2, 42.25f)
        << "Cs" << IqRibParser::TqFloatArray(4, 2.0f);
}


//--------------------------------------------------
RtVoid MockRenderer::Color(RtColor col)
{
    CheckParams() << Req("Color") << col[0] << col[1] << col[2];
}
BOOST_AUTO_TEST_CASE(Color_handler_test)
{
    RequestHandlerFixture f;
    Insert(f.parser) << Req("Color") << 1.0f << 2.0f << 3.0f;
}


//--------------------------------------------------
RtVoid MockRenderer::PixelFilter(RtFilterFunc function, RtFloat xwidth, RtFloat ywidth)
{
    BOOST_CHECK_EQUAL(function, RiSincFilter);
    CheckParams() << Req("PixelFilter") << IgnoreParam() << xwidth << ywidth;
}
BOOST_AUTO_TEST_CASE(PixelFilter_handler_test)
{
    RequestHandlerFixture f;
    Insert(f.parser) << Req("PixelFilter") << "sinc" << 7.0f << 7.5f;
}
BOOST_AUTO_TEST_CASE(PixelFilter_bad_filter_name)
{
    RequestHandlerFixture f;
    BOOST_CHECK_THROW(
        Insert(f.parser) << Req("PixelFilter") << "nonexistent" << 7.0f << 7.5f,
        XqParseError
    );
}


//--------------------------------------------------
RtVoid MockRenderer::ErrorHandler(RtErrorFunc handler)
{
    BOOST_CHECK_EQUAL(handler, RiErrorAbort);
    CheckParams() << Req("ErrorHandler");
}
BOOST_AUTO_TEST_CASE(ErrorFunc_handler_test)
{
    RequestHandlerFixture f;
    Insert(f.parser) << Req("ErrorHandler") << "abort";
}
BOOST_AUTO_TEST_CASE(ErrorFunc_bad_error_handler_name)
{
    RequestHandlerFixture f;
    BOOST_CHECK_THROW(
        Insert(f.parser) << Req("ErrorHandler") << "bogus",
        XqParseError
    );
}


//--------------------------------------------------
RtVoid MockRenderer::Transform(RtMatrix transform)
{
    float* trans = reinterpret_cast<float*>(transform);
    CheckParams() << Req("Transform")
        << IqRibParser::TqFloatArray(trans, trans+16);
}
BOOST_AUTO_TEST_CASE(Transform_handler_test)
{
    float trans[16] = {
        1,2,3,4,
        5,6,7,8,
        9,-1,-2,-3,
        -4,-5,-6,-7
    };

    RequestHandlerFixture f;
    Insert(f.parser) << Req("Transform")
        << IqRibParser::TqFloatArray(trans, trans+16);
}

RtVoid MockRenderer::Points(RtInt npoints, RtInt count, RtToken tokens[], RtPointer values[])
{
    // This check will only work for the specific case below
    BOOST_CHECK_EQUAL(npoints, 4);
    CheckParams() << Req("Points") << ParamList(count, tokens, values);
}
BOOST_AUTO_TEST_CASE(Points_handler_test)
{
    RequestHandlerFixture f;
    Insert(f.parser) << Req("Points")
        << "Cs" << IqRibParser::TqFloatArray(12, 0.5f)
        << "P" << IqRibParser::TqFloatArray(12, 1.5f);
}
BOOST_AUTO_TEST_CASE(Points_missing_P_variable)
{
    RequestHandlerFixture f;
    BOOST_CHECK_THROW(
        Insert(f.parser) << Req("Points")
            << "Cs" << IqRibParser::TqFloatArray(12, 0.5f),
        XqParseError
    );
}


//--------------------------------------------------
RtVoid MockRenderer::MotionBegin(RtInt N, RtFloat times[])
{
    CheckParams() << Req("MotionBegin")
        << IqRibParser::TqFloatArray(times, times+N);
}
BOOST_AUTO_TEST_CASE(MotionBegin_handler_test)
{
    RequestHandlerFixture f;
    IqRibParser::TqFloatArray times;
    times += 0, 1, 2.5, 3, 4;
    Insert(f.parser) << Req("MotionBegin") << times;
}


//--------------------------------------------------
RtVoid MockRenderer::MakeOcclusion(RtInt npics, RtString picfiles[], RtString shadowfile,
        RtInt count, RtToken tokens[], RtPointer values[])
{
    CheckParams() << Req("MakeOcclusion")
        << IqRibParser::TqStringArray(picfiles, picfiles+npics)
        << shadowfile << ParamList(count, tokens, values);
}
BOOST_AUTO_TEST_CASE(MakeOcclusion_handler_test)
{
    IqRibParser::TqStringArray picFiles;
    picFiles += "pic1", "pic2", "pic3", "some_pic4", "asdf5", "Occlmap6.om";
    RequestHandlerFixture f;
    Insert(f.parser) << Req("MakeOcclusion") << picFiles << "ambient.map";
}


//------------------------------------------------------------------------------
// Test for parameter list handling, using RiOption as a proxy.
//
RtVoid MockRenderer::Option(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
    CheckParams() << Req("Option") << name << ParamList(count, tokens, values);
    // Check that the parameter list token count is correct.  Note that the
    // formula used below assumes there's only one Req in the token stream.
    BOOST_CHECK_EQUAL(2*count, static_cast<int>(g_fixture->parser.params().size()) - 2);
}

BOOST_AUTO_TEST_CASE(paramlist_int_array_value_test)
{
    RequestHandlerFixture f;
    Insert(f.parser) << Req("Option") << "some_option_name"
        << "uniform int asdf" << IqRibParser::TqIntArray(10, 42);
}
BOOST_AUTO_TEST_CASE(paramlist_float_array_value_test)
{
    RequestHandlerFixture f;
    Insert(f.parser) << Req("Option") << "some_option_name"
        << "uniform float asdf" << IqRibParser::TqFloatArray(10, 2.5f);
}
BOOST_AUTO_TEST_CASE(paramlist_string_array_value_test)
{
    RequestHandlerFixture f;
    Insert(f.parser) << Req("Option") << "some_option_name"
        << "uniform string asdf" << IqRibParser::TqStringArray(10, "asdf_value");
}
BOOST_AUTO_TEST_CASE(extended_paramlist_test)
{
    RequestHandlerFixture f;
    // Check that many mixed params work fine.
    Insert(f.parser) << Req("Option") << "some_option_name"
        << "uniform int user_i" << IqRibParser::TqIntArray(1, 42)
        << "uniform float user_f" << IqRibParser::TqFloatArray(1, 2.5f)
        << "uniform vector user_v" << IqRibParser::TqFloatArray(3, 3.5f)
        << "uniform matrix user_m" << IqRibParser::TqFloatArray(16, 4.5f)
        << "uniform string s1" << IqRibParser::TqStringArray(1, "blah")
        << "uniform string s2" << IqRibParser::TqStringArray(1, "s2val")
        << "uniform string s3" << IqRibParser::TqStringArray(1, "s3val")
        << "uniform string s4" << IqRibParser::TqStringArray(1, "s4val");
}

BOOST_AUTO_TEST_CASE(invalid_paramlist_unknown_primvar)
{
    // Check that unknown primvar names throw.
    RequestHandlerFixture f;
    BOOST_CHECK_THROW(
        Insert(f.parser) << Req("Option") << "some_option_name"
            << "asdf" << IqRibParser::TqFloatArray(1, 42.25f),
        XqParseError
    );
}
BOOST_AUTO_TEST_CASE(invalid_paramlist_invalid_primvar)
{
    // Check that invalid primvar strings throw.
    RequestHandlerFixture f;
    BOOST_CHECK_THROW(
        Insert(f.parser) << Req("Option") << "some_option_name"
            << "] bad_token" << IqRibParser::TqFloatArray(1, 42.25f),
        XqParseError
    );
}
BOOST_AUTO_TEST_CASE(invalid_paramlist_missing_token)
{
    // check that a missing token throws.
    RequestHandlerFixture f;
    BOOST_CHECK_THROW(
        Insert(f.parser) << Req("Option") << "some_option_name"
            << "P" << IqRibParser::TqFloatArray(4, 42.25f)
            << IqRibParser::TqIntArray(1, 42),
        XqParseError
    );
}

#endif
