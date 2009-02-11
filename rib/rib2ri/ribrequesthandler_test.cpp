#include "ribrequesthandler.h"

#include <cstddef>
#include <sstream>

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <boost/assign/std/vector.hpp>

#include "ri.h"
#include "iribparser.h"

using namespace boost::assign; // necessary for container initialisation operators.

using namespace Aqsis;


//------------------------------------------------------------------------------

template<typename T>
bool arrayEqual(const std::vector<T>& v1, const void* v2)
{
	const T* v2T = reinterpret_cast<const T*>(v2);
	for(TqInt i = 0, end = v1.size(); i < end; ++i)
	{
		if(v1[i] != v2T[i])
			return false;
	}
	return true;
}

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
/** Cheap and nasty variant holding tokens for CqMockParser
 *
 * Yeah, boost::variant would allow me to avoid implementing this class, but
 * the error messages are absolutely insane, to the point of making the code
 * unmaintainable.
 */

enum EqType
{
	Type_Int,
	Type_Float,
	Type_String,
	Type_IntArray,
	Type_FloatArray,
	Type_StringArray,
	Type_Request,
};

class CqMockRibToken
{
	private:
		EqType m_type;

		TqInt m_int;
		TqFloat m_float;
		std::string m_string;
		Req m_request;
		IqRibParser::TqIntArray m_ints;
		IqRibParser::TqFloatArray m_floats;
		IqRibParser::TqStringArray m_strings;

	public:
		CqMockRibToken(TqInt i)
			: m_type(Type_Int), m_int(i) { }
		CqMockRibToken(TqFloat f)
			: m_type(Type_Float), m_float(f) { }
		CqMockRibToken(const char* s)
			: m_type(Type_String), m_string(s) { }
		CqMockRibToken(const std::string& s)
			: m_type(Type_String), m_string(s) { }
		CqMockRibToken(const Req& request)
			: m_type(Type_Request), m_request(request) { }
		CqMockRibToken(const IqRibParser::TqIntArray& v)
			: m_type(Type_IntArray), m_ints(v) { }
		CqMockRibToken(const IqRibParser::TqFloatArray& v)
			: m_type(Type_FloatArray), m_floats(v) { }
		CqMockRibToken(const IqRibParser::TqStringArray& v)
			: m_type(Type_StringArray), m_strings(v) { }

		EqType type() const { return m_type; }

		TqInt getInt() const
				{ assert(m_type == Type_Int); return m_int; }
		TqFloat getFloat() const
				{ assert(m_type == Type_Float); return m_float; }
		const std::string& getString() const
				{ assert(m_type == Type_String); return m_string; }
		const Req& getReq() const
				{ assert(m_type == Type_Request); return m_request; }
		const IqRibParser::TqIntArray& getIntArray() const
				{ assert(m_type == Type_IntArray); return m_ints; }
		const IqRibParser::TqFloatArray& getFloatArray() const
				{ assert(m_type == Type_FloatArray); return m_floats; }
		const IqRibParser::TqStringArray& getStringArray() const
				{ assert(m_type == Type_StringArray); return m_strings; }

		bool operator==(const CqMockRibToken& rhs) const
		{
			if(m_type != rhs.m_type)
				return false;
			switch(m_type)
			{
				case Type_Int:			return getInt() == rhs.getInt();
				case Type_Float:		return getFloat() == rhs.getFloat();
				case Type_String:		return getString() == rhs.getString();
				case Type_IntArray:		return getIntArray() == rhs.getIntArray();
				case Type_FloatArray:	return getFloatArray() == rhs.getFloatArray();
				case Type_StringArray:	return getStringArray() == rhs.getStringArray();
				case Type_Request:		return getReq() == rhs.getReq();
			}
			assert(0 && "unrecognised type??");
			return false;
		}

};

std::ostream& operator<<(std::ostream& out, const CqMockRibToken& tok)
{
	switch(tok.type())
	{
		case Type_Int: 			out << tok.getInt();		break;
		case Type_Float: 		out << tok.getFloat();		break;
		case Type_String: 		out << tok.getString();		break;
		case Type_IntArray: 	out << tok.getIntArray();	break;
		case Type_FloatArray: 	out << tok.getFloatArray();	break;
		case Type_StringArray: 	out << tok.getStringArray();break;
		case Type_Request: 		out << tok.getReq();		break;
	}
	return out;
}

// Compare a token containing an array to a raw C array.
//
// assert() if the token doesn't contain an array.
bool operator==(const CqMockRibToken& lhs, const void* rhs)
{
	switch(lhs.type())
	{
		case Type_IntArray:		return arrayEqual(lhs.getIntArray(), rhs);
		case Type_FloatArray:	return arrayEqual(lhs.getFloatArray(), rhs);
		case Type_StringArray:	return arrayEqual(lhs.getStringArray(), rhs);
		default:
			assert(0 && "CqMockRibToken void* compare not implemented for type");
	}
	return false;
}


//------------------------------------------------------------------------------
// Test fixtures, mock objects and checking functions.

typedef std::vector<CqMockRibToken> TqTokVec;

// Mock implementation of the RIB parser interface for testing purposes.
//
// This "parser" uses a vector of boost::any to describe the input which
// should be provided to the request handler object.  When data is requested,
// the mock parser just uses boost::any_cast to cast the current input element
// to the requested type; an exception is thrown if the types don't match.
//
// The input data should be provided via the params() accessor before an
// attempt is made to use the parser.
class CqMockParser : public IqRibParser
{
	private:
		TqTokVec m_params;
		CqRibRequestHandler& m_handler;
		TqInt m_tokenPos;
		TqInt m_currentRequestStart;
		std::vector<TqFloatArray> m_floatVecStorage;
	public:
		CqMockParser(CqRibRequestHandler& handler)
			: m_params(),
			m_handler(handler),
			m_tokenPos(0),
			m_currentRequestStart(0),
			m_floatVecStorage()
		{ }

		TqTokVec& params()
		{
			return m_params;
		}

		TqTokVec::const_iterator currParams() const
		{
			return m_params.begin() + m_currentRequestStart;
		}

		TqInt currentRequestStart() const
		{
			return m_currentRequestStart;
		}

		virtual bool parseNextRequest()
		{
			m_currentRequestStart = m_tokenPos;
			m_handler.handleRequest(m_params[m_tokenPos++].getReq().name, *this );
			return false;
		}

		virtual void pushInput(std::istream& inStream, const std::string& name) {}
		virtual void popInput() {}

		virtual TqInt getInt()
		{
			return m_params[m_tokenPos++].getInt();
		}
		virtual TqFloat getFloat()
		{
			return m_params[m_tokenPos++].getFloat();
		}
		virtual std::string getString()
		{
			return m_params[m_tokenPos++].getString();
		}

		virtual const TqIntArray& getIntArray()
		{
			return m_params[m_tokenPos++].getIntArray();
		}
		virtual const TqFloatArray& getFloatArray(TqInt length = -1)
		{
			if(length >= 0 && m_params[m_tokenPos].type() == Type_Float)
			{
				m_floatVecStorage.push_back(TqFloatArray(length,0));
				// read in individual floats for the array
				TqFloatArray& array = m_floatVecStorage.back();
				for(TqInt i = 0; i < length; ++i)
					array[i] = m_params[m_tokenPos++].getFloat();
				return array;
			}
			else
			{
				const TqFloatArray& array = m_params[m_tokenPos++].getFloatArray();
				if(length >= 0 && static_cast<TqInt>(array.size()) != length)
					throw std::runtime_error("Bad array length detected.");
				return array;
			}
		}
		virtual const TqStringArray& getStringArray()
		{
			return m_params[m_tokenPos++].getStringArray();
		}

		virtual void getParamList(IqRibParamListHandler& paramHandler)
		{
			while(m_tokenPos < static_cast<TqInt>(m_params.size())
					&& m_params[m_tokenPos].type() != Type_Request)
				paramHandler.readParameter(getString(), *this);
		}

		virtual EqRibToken peekNextType()
		{
			if(m_tokenPos >= static_cast<TqInt>(m_params.size()))
				return Tok_RequestEnd;
			switch(m_params[m_tokenPos].type())
			{
				case Type_Int:			return Tok_Int;
				case Type_Float:		return Tok_Float;
				case Type_String:		return Tok_String;
				case Type_IntArray:
				case Type_FloatArray:
				case Type_StringArray:	return Tok_Array;
				case Type_Request:		return Tok_RequestEnd;
			}
			assert(0 &&"peekNextType - type unknown");
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

/** Manage mappings from object or light handles to pointers for testing.
 *
 * If we want to check that the mapping is happening correctly we need to
 * record both the RIB object id and the returned pointer.
 */
class CqHandleManager
{
	private:
		TqInt m_currHandle;
		typedef std::vector<std::pair<CqMockRibToken, RtPointer> > TqHandleMap;
		TqHandleMap m_handleMap;

	public:
		CqHandleManager()
			: m_currHandle(0),
			m_handleMap()
		{ }

		RtPointer insertHandle(const CqMockRibToken& id)
		{
			assert(id.type() == Type_Int
					|| id.type() == Type_String);
			RtPointer p = reinterpret_cast<RtPointer>(++m_currHandle);
			m_handleMap.push_back(std::make_pair(id, p));
			return p;
		}
		RtPointer lookup(const CqMockRibToken& id) const
		{
			assert(id.type() == Type_String || id.type() == Type_Int);
			for(TqHandleMap::const_iterator i = m_handleMap.begin();
					i < m_handleMap.end(); ++i)
			{
				if(id == i->first)
					return i->second;
			}
			throw std::runtime_error("handle not found");
			return 0;
		}
};

struct SqRequestHandlerFixture
{
	// Object to be tested
	CqRibRequestHandler handler;
	// Mock parser
	CqMockParser parser;
	// Flag indicating that a request has been checked.
	bool hasBeenChecked;
	// Manager for holding handles
	CqHandleManager handleManager;

	SqRequestHandlerFixture();
	~SqRequestHandlerFixture();

	void setChecked()
	{
		hasBeenChecked = true;
	}
};

#define FIXTURE_CHECKED(f) \
	do { BOOST_CHECK(f.hasBeenChecked); f.hasBeenChecked = false; } while(false)

// global to enable Ri* function implementations to validate the function
// parameters against those which were input to the handler via the parser.
static SqRequestHandlerFixture* g_fixture = 0;

SqRequestHandlerFixture::SqRequestHandlerFixture()
	: handler(),
	parser(handler),
	hasBeenChecked(false)
{
	g_fixture = this;
}

SqRequestHandlerFixture::~SqRequestHandlerFixture()
{
	g_fixture = 0;
}

// Check equality of paramters with the input parameters held in the g_fixture
// global struct.
void checkParams(const TqTokVec& positionalParams,
		RtInt count, RtToken tokens[], RtPointer values[])
{
	BOOST_REQUIRE(g_fixture != 0);

	TqInt numPosParams = positionalParams.size();
	TqInt numParams = numPosParams + count;
	TqInt start = g_fixture->parser.currentRequestStart();
	TqTokVec::const_iterator p = g_fixture->parser.currParams();
	BOOST_REQUIRE(start + numParams <=
			static_cast<TqInt>(g_fixture->parser.params().size()));
	// Check that all positional parameters match up.
	for(TqInt i = 0; i < numPosParams; ++i)
		BOOST_CHECK_EQUAL(*(p++), positionalParams[i]);

	// Check that all parameter-list parameters match up.
	for(TqInt i = 0; i < count; ++i)
	{
		BOOST_CHECK_EQUAL((p++)->getString(), tokens[i]);
		BOOST_CHECK_EQUAL(*(p++), values[i]);
	}

	g_fixture->setChecked();
}



//==============================================================================
// The actual test cases follow below.
//==============================================================================

//------------------------------------------------------------------------------
// Tests for handler functions with hand-written implementations.


RtToken RiDeclare(RtString name, RtString declaration)
{
	TqTokVec posParams; posParams += Req("Declare"), name, declaration;
	checkParams(posParams, 0, 0, 0);
	g_fixture->setChecked();
	return 0;
}
BOOST_AUTO_TEST_CASE(RiDeclare_handler_test)
{
	SqRequestHandlerFixture f;
	f.parser.params() +=
		Req("Declare"), "asdf", "uniform float",
		Req("Sphere"), 1.0f, -1.0f, 1.0f, 360.0f,
			"asdf", IqRibParser::TqFloatArray(1, 42.0f);
	f.parser.parseNextRequest();
	FIXTURE_CHECKED(f);
	f.parser.parseNextRequest();
	FIXTURE_CHECKED(f);
}


RtVoid RiDepthOfField(RtFloat fstop, RtFloat focallength, RtFloat focaldistance)
{
	if(fstop == FLT_MAX)
	{
		BOOST_CHECK_EQUAL(g_fixture->parser.params().size(), 1U);
		BOOST_CHECK_EQUAL(focallength, FLT_MAX);
		BOOST_CHECK_EQUAL(focaldistance, FLT_MAX);
		g_fixture->setChecked();
	}
	else
	{
		TqTokVec posParams;
		posParams += Req("DepthOfField"), fstop, focallength, focaldistance;
		checkParams(posParams, 0, 0, 0);
	}
}
BOOST_AUTO_TEST_CASE(RiDepthOfField_handler_test)
{
	{
		SqRequestHandlerFixture f;
		f.parser.params() += Req("DepthOfField"), 1.0f, 42.0f, 42.5f;
		f.parser.parseNextRequest();
		FIXTURE_CHECKED(f);
	}
	{
		SqRequestHandlerFixture f;
		f.parser.params() += Req("DepthOfField");
		f.parser.parseNextRequest();
		FIXTURE_CHECKED(f);
	}
}


RtVoid RiColorSamples(RtInt N, RtFloat nRGB[], RtFloat RGBn[])
{
	TqTokVec posParams; posParams += Req("ColorSamples"),
			 IqRibParser::TqFloatArray(nRGB, nRGB + 3*N),
			 IqRibParser::TqFloatArray(RGBn, RGBn + 3*N);
	checkParams(posParams, 0, 0, 0);
}
BOOST_AUTO_TEST_CASE(RiColorSamples_handler_test)
{
	SqRequestHandlerFixture f;
	f.parser.params() += Req("ColorSamples"), IqRibParser::TqFloatArray(12, 1.0f),
		IqRibParser::TqFloatArray(12, 1.0f);
	f.parser.parseNextRequest();
	FIXTURE_CHECKED(f);
}


RtLightHandle RiLightSourceV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
	TqTokVec posParams;
	posParams += Req("LightSource"), name;
	// this is a slight abuse of checkParams, since the RIB and C interfaces
	// don't match.  It'll fail for nonempty param lists :-/
	checkParams(posParams, count, tokens, values);
	return g_fixture->handleManager.insertHandle(g_fixture->parser.currParams()[2]);
}
BOOST_AUTO_TEST_CASE(RiLightSource_handler_test)
{
	{
		// Test integer light identifiers
		SqRequestHandlerFixture f;
		f.parser.params() += Req("LightSource"), "blahlight", 10;
		f.parser.parseNextRequest();
		FIXTURE_CHECKED(f);
	}
	{
		// Test string light identifiers
		SqRequestHandlerFixture f;
		f.parser.params() += Req("LightSource"), "blahlight", "string_index";
		f.parser.parseNextRequest();
		FIXTURE_CHECKED(f);
	}
}


RtVoid RiIlluminate(RtLightHandle light, RtBoolean onoff)
{
	TqTokVec::const_iterator p = g_fixture->parser.currParams();
	BOOST_CHECK_EQUAL(*(p++), Req("Illuminate"));
	BOOST_CHECK_EQUAL(g_fixture->handleManager.lookup(*(p++)), light);
	BOOST_CHECK_EQUAL(*(p++), static_cast<int>(onoff));
	g_fixture->hasBeenChecked = true;
}
BOOST_AUTO_TEST_CASE(RiIlluminate_handler_test)
{
	{
		SqRequestHandlerFixture f;
		f.parser.params() += Req("Illuminate"), 10, 1;
		BOOST_CHECK_THROW(f.parser.parseNextRequest(), XqParseError);
	}
	{
		SqRequestHandlerFixture f;
		f.parser.params() +=
			Req("LightSource"), "blahlight", 10,
			Req("Illuminate"), 10, 0,
			Req("LightSource"), "asdflight", 11,
			Req("Illuminate"), 11, 1;
		f.parser.parseNextRequest();
		FIXTURE_CHECKED(f);
		f.parser.parseNextRequest();
		FIXTURE_CHECKED(f);
		f.parser.parseNextRequest();
		FIXTURE_CHECKED(f);
		f.parser.parseNextRequest();
		FIXTURE_CHECKED(f);
	}
}


RtVoid RiBasis(RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep)
{
	const TqTokVec& params = g_fixture->parser.params();
	BOOST_CHECK_EQUAL(params[0].getReq().name, "Basis");
	BOOST_CHECK_EQUAL(::RiBSplineBasis, ubasis);
	BOOST_CHECK_EQUAL(params[2].getInt(), ustep);
	BOOST_CHECK_EQUAL(params[3], vbasis);
	BOOST_CHECK_EQUAL(params[4].getInt(), vstep);
	g_fixture->setChecked();
}
BOOST_AUTO_TEST_CASE(RiBasis_handler_test)
{
	SqRequestHandlerFixture f;
	f.parser.params() += Req("Basis"), "b-spline", 1, IqRibParser::TqFloatArray(16, 2.0f), 42;
	f.parser.parseNextRequest();
	FIXTURE_CHECKED(f);
}


RtVoid RiSubdivisionMeshV(RtToken scheme, RtInt nfaces, RtInt nvertices[],
		RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[],
		RtInt intargs[], RtFloat floatargs[],
		RtInt count, RtToken tokens[], RtPointer values[])
{
}


RtVoid RiHyperboloidV(RtPoint point1, RtPoint point2, RtFloat thetamax,
		RtInt count, RtToken tokens[], RtPointer values[])
{
	TqTokVec posParams; posParams += Req("Hyperboloid"), point1[0], point1[1], point1[2],
		point2[0], point2[1], point2[2], thetamax;
	checkParams(posParams, count, tokens, values);
}
BOOST_AUTO_TEST_CASE(RiHyperboloidV_handler_test)
{
	SqRequestHandlerFixture f;
	f.parser.params() += Req("Hyperboloid"), 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 360.0f;
	f.parser.parseNextRequest();
	FIXTURE_CHECKED(f);
}


RtVoid RiProcedural(RtPointer data, RtBound bound, RtProcSubdivFunc refineproc, RtProcFreeFunc freeproc)
{
}


RtObjectHandle RiObjectBegin()
{
	return 0;
}


RtVoid RiObjectInstance(RtObjectHandle handle)
{
}


//------------------------------------------------------------------------------
// Tests for some of the autogenerated handler implementations.  Full test
// coverage would be quite a burden (there's almost 100 interface functions!),
// so only a few are tested here.  The hope is that they provide sufficient
// coverage of the code generator...




RtVoid RiSphereV(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax,
		RtInt count, RtToken tokens[], RtPointer values[])
{
	TqTokVec posParams; posParams += Req("Sphere"), radius, zmin, zmax, thetamax;
	checkParams(posParams, count, tokens, values);
}
BOOST_AUTO_TEST_CASE(RiSphereV_handler_test)
{
	// Test RiSphere; also test the handling of valid parameter lists.
	SqRequestHandlerFixture f;
	f.parser.params() += Req("Sphere"), 2.5f, -1.0f, 1.0f, 360.0f,
		"uniform float blah", IqRibParser::TqFloatArray(2, 42.25f),
		"Cs", IqRibParser::TqFloatArray(4, 2.0f);
	// std::cerr << "Checking --- " << f.parser.params() << "\n";
	f.parser.parseNextRequest();
	FIXTURE_CHECKED(f);
}


BOOST_AUTO_TEST_CASE(invalid_paramlist_handling)
{
	{
		// Check that unknown primvar names throw.
		SqRequestHandlerFixture f;
		f.parser.params() += Req("Sphere"), 1.0f, 1.0f, 1.0f, 1.0f,
			"asdf", IqRibParser::TqFloatArray(1, 42.25f);
		//try{ f.parser.parseNextRequest(); }
		//catch(XqParseError& e) { std::cerr << e.what() << "\n"; }
		BOOST_CHECK_THROW(f.parser.parseNextRequest(), XqParseError);
	}
	{
		// Check that invalid primvar strings throw.
		SqRequestHandlerFixture f;
		f.parser.params() += Req("Sphere"), 1.0f, 1.0f, 1.0f, 1.0f,
			"] bad_token", IqRibParser::TqFloatArray(1, 42.25f);
		BOOST_CHECK_THROW(f.parser.parseNextRequest(), XqParseError);
	}
}



//==============================================================================
// Definitions of RI symbols

// bases
RtBasis RiBezierBasis =
{
	{ -1,  3, -3, 1},
	{  3, -6,  3, 0},
	{ -3,  3,  0, 0},
	{  1,  0,  0, 0}
};
RtBasis RiBSplineBasis =
{
	{ -1.0 / 6,  3.0 / 6, -3.0 / 6, 1.0 / 6},
	{  3.0 / 6, -6.0 / 6,  3.0 / 6, 0.0 / 6},
	{ -3.0 / 6,  0.0 / 6,  3.0 / 6, 0.0 / 6},
	{  1.0 / 6,  4.0 / 6,  1.0 / 6, 0.0 / 6}
};
RtBasis RiCatmullRomBasis =
{
	{ -1.0 / 2,  3.0 / 2, -3.0 / 2,  1.0 / 2},
	{  2.0 / 2, -5.0 / 2,  4.0 / 2, -1.0 / 2},
	{ -1.0 / 2,  0.0 / 2,  1.0 / 2,  0.0 / 2},
	{  0.0 / 2,  2.0 / 2,  0.0 / 2,  0.0 / 2}
};
RtBasis RiHermiteBasis =
{
	{  2,  1, -2,  1},
	{ -3, -2,  3, -1},
	{  0,  1,  0,  0},
	{  1,  0,  0,  0}
};
RtBasis RiPowerBasis =
{
	{ 1, 0, 0, 0},
	{ 0, 1, 0, 0},
	{ 0, 0, 1, 0},
	{ 0, 0, 0, 1}
};


// Empty implementations of all required Ri* functions.
// Autogenerated via XSLT, with those functions actually used above commented out.

//RtToken RiDeclare(RtString name, RtString declaration) {return 0;}
RtVoid RiBegin(RtToken name) {}
RtVoid RiEnd() {}
RtContextHandle RiGetContext() {return 0;}
RtVoid RiContext(RtContextHandle handle) {}
RtVoid RiFrameBegin(RtInt number) {}
RtVoid RiFrameEnd() {}
RtVoid RiWorldBegin() {}
RtVoid RiWorldEnd() {}
RtVoid RiIfBegin(RtString condition) {}
RtVoid RiElseIf(RtString condition) {}
RtVoid RiElse() {}
RtVoid RiIfEnd() {}
RtVoid RiFormat(RtInt xresolution, RtInt yresolution, RtFloat pixelaspectratio) {}
RtVoid RiFrameAspectRatio(RtFloat frameratio) {}
RtVoid RiScreenWindow(RtFloat left, RtFloat right, RtFloat bottom, RtFloat top) {}
RtVoid RiCropWindow(RtFloat xmin, RtFloat xmax, RtFloat ymin, RtFloat ymax) {}
RtVoid RiProjectionV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiClipping(RtFloat cnear, RtFloat cfar) {}
RtVoid RiClippingPlane(RtFloat x, RtFloat y, RtFloat z, RtFloat nx, RtFloat ny, RtFloat nz) {}
//RtVoid RiDepthOfField(RtFloat fstop, RtFloat focallength, RtFloat focaldistance) {}
RtVoid RiShutter(RtFloat opentime, RtFloat closetime) {}
RtVoid RiPixelVariance(RtFloat variance) {}
RtVoid RiPixelSamples(RtFloat xsamples, RtFloat ysamples) {}
RtVoid RiPixelFilter(RtFilterFunc function, RtFloat xwidth, RtFloat ywidth) {}
RtVoid RiExposure(RtFloat gain, RtFloat gamma) {}
RtVoid RiImagerV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiQuantize(RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ditheramplitude) {}
RtVoid RiDisplayV(RtToken name, RtToken type, RtToken mode, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtFloat RiGaussianFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) {return 0;}
RtFloat RiBoxFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) {return 0;}
RtFloat RiMitchellFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) {return 0;}
RtFloat RiTriangleFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) {return 0;}
RtFloat RiCatmullRomFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) {return 0;}
RtFloat RiSincFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) {return 0;}
RtFloat RiDiskFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) {return 0;}
RtFloat RiBesselFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) {return 0;}
RtVoid RiHiderV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
//RtVoid RiColorSamples(RtInt N, RtFloat nRGB[], RtFloat RGBn[]) {}
RtVoid RiRelativeDetail(RtFloat relativedetail) {}
RtVoid RiOptionV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiAttributeBegin() {}
RtVoid RiAttributeEnd() {}
RtVoid RiColor(RtColor Cq) {}
RtVoid RiOpacity(RtColor Os) {}
RtVoid RiTextureCoordinates(RtFloat s1, RtFloat t1, RtFloat s2, RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4, RtFloat t4) {}
//RtLightHandle RiLightSourceV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {return 0;}
RtLightHandle RiAreaLightSourceV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {return 0;}
//RtVoid RiIlluminate(RtLightHandle light, RtBoolean onoff) {}
RtVoid RiSurfaceV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiDeformationV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiDisplacementV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiAtmosphereV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiInteriorV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiExteriorV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiShaderLayerV(RtToken type, RtToken name, RtToken layername, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiConnectShaderLayers(RtToken type, RtToken layer1, RtToken variable1, RtToken layer2, RtToken variable2) {}
RtVoid RiShadingRate(RtFloat size) {}
RtVoid RiShadingInterpolation(RtToken type) {}
RtVoid RiMatte(RtBoolean onoff) {}
RtVoid RiBound(RtBound bound) {}
RtVoid RiDetail(RtBound bound) {}
RtVoid RiDetailRange(RtFloat offlow, RtFloat onlow, RtFloat onhigh, RtFloat offhigh) {}
RtVoid RiGeometricApproximation(RtToken type, RtFloat value) {}
RtVoid RiOrientation(RtToken orientation) {}
RtVoid RiReverseOrientation() {}
RtVoid RiSides(RtInt nsides) {}
RtVoid RiIdentity() {}
RtVoid RiTransform(RtMatrix transform) {}
RtVoid RiConcatTransform(RtMatrix transform) {}
RtVoid RiPerspective(RtFloat fov) {}
RtVoid RiTranslate(RtFloat dx, RtFloat dy, RtFloat dz) {}
RtVoid RiRotate(RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz) {}
RtVoid RiScale(RtFloat sx, RtFloat sy, RtFloat sz) {}
RtVoid RiSkew(RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1, RtFloat dx2, RtFloat dy2, RtFloat dz2) {}
RtVoid RiCoordinateSystem(RtToken space) {}
RtVoid RiCoordSysTransform(RtToken space) {}
RtPoint* RiTransformPoints(RtToken fromspace, RtToken tospace, RtInt npoints, RtPoint points[]) {return 0;}
RtVoid RiTransformBegin() {}
RtVoid RiTransformEnd() {}
RtVoid RiResourceV(RtToken handle, RtToken type, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiResourceBegin() {}
RtVoid RiResourceEnd() {}
RtVoid RiAttributeV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiPolygonV(RtInt nvertices, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiGeneralPolygonV(RtInt nloops, RtInt nverts[], RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiPointsPolygonsV(RtInt npolys, RtInt nverts[], RtInt verts[], RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiPointsGeneralPolygonsV(RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], RtInt count, RtToken tokens[], RtPointer values[]) {}
//RtVoid RiBasis(RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep) {}
RtVoid RiPatchV(RtToken type, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiPatchMeshV(RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiNuPatchV(RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiTrimCurve(RtInt nloops, RtInt ncurves[], RtInt order[], RtFloat knot[], RtFloat min[], RtFloat max[], RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[]) {}
//RtVoid RiSubdivisionMeshV(RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], RtInt count, RtToken tokens[], RtPointer values[]) {}
//RtVoid RiSphereV(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiConeV(RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiCylinderV(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) {}
//RtVoid RiHyperboloidV(RtPoint point1, RtPoint point2, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiParaboloidV(RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiDiskV(RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiTorusV(RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiPointsV(RtInt npoints, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiCurvesV(RtToken type, RtInt ncurves, RtInt nvertices[], RtToken wrap, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiBlobbyV(RtInt nleaf, RtInt ncode, RtInt code[], RtInt nflt, RtFloat flt[], RtInt nstr, RtToken str[], RtInt count, RtToken tokens[], RtPointer values[]) {}
//RtVoid RiProcedural(RtPointer data, RtBound bound, RtProcSubdivFunc refineproc, RtProcFreeFunc freeproc) {}
RtVoid RiProcFree(RtPointer data) {}
RtVoid RiProcDelayedReadArchive(RtPointer data, RtFloat detail) {}
RtVoid RiProcRunProgram(RtPointer data, RtFloat detail) {}
RtVoid RiProcDynamicLoad(RtPointer data, RtFloat detail) {}
RtVoid RiGeometryV(RtToken type, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiSolidBegin(RtToken type) {}
RtVoid RiSolidEnd() {}
//RtObjectHandle RiObjectBegin() {return 0;}
RtVoid RiObjectEnd() {}
//RtVoid RiObjectInstance(RtObjectHandle handle) {}
RtVoid RiMotionBegin(RtInt N,  ...) {}
RtVoid RiMotionBeginV(RtInt N, RtFloat times[]) {}
RtVoid RiMotionEnd() {}
RtVoid RiMakeTextureV(RtString imagefile, RtString texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiMakeBumpV(RtString imagefile, RtString bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiMakeLatLongEnvironmentV(RtString imagefile, RtString reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiMakeCubeFaceEnvironmentV(RtString px, RtString nx, RtString py, RtString ny, RtString pz, RtString nz, RtString reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiMakeShadowV(RtString picfile, RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiMakeOcclusionV(RtInt npics, RtString picfiles[], RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiErrorHandler(RtErrorFunc handler) {}
RtVoid RiErrorIgnore(RtInt code, RtInt severity, RtString message) {}
RtVoid RiErrorPrint(RtInt code, RtInt severity, RtString message) {}
RtVoid RiErrorAbort(RtInt code, RtInt severity, RtString message) {}
RtVoid RiArchiveRecord(RtToken type, char * format,  ...) {}
RtVoid RiReadArchiveV(RtToken name, RtArchiveCallback callback, RtInt count, RtToken tokens[], RtPointer values[]) {}
