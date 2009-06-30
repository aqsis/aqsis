#include <aqsis/aqsis.h>

#ifndef	AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

#include "ribrequesthandler.h"

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
// Unfortunately, writing tests for CqRibRequestHandler easily and elegantly
// requires a *lot* of setup.  At the centre of it all is a mock object
// CqMockParser which implements the IqRibParser interface and acts as the
// callback object for CqRibRequestHandler during testing.
//
// The machinery below exists to help the insertion of valid token sequences
// into CqMockParser and to check those sequences against the output which
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

// Types for the current element of CqMockParser.
enum EqType
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

// Tag struct to indicate an empty CqMockRibToken, used to reconcile differing
// RIB and RI forms of requests where necessary.
struct IgnoreParam {};

/** Cheap and nasty variant holding tokens for CqMockParser
 *
 * Yeah, boost::variant would allow me to avoid implementing this class, but
 * the error messages are absolutely insane, to the point of making the code
 * unmaintainable.
 */
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
		CqMockRibToken(const IgnoreParam&)
			: m_type(Type_Ignore) { }

		EqType type() const { return m_type; }

		TqInt getInt() const
				{ BOOST_REQUIRE(m_type == Type_Int); return m_int; }
		TqFloat getFloat() const
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

		bool operator==(const CqMockRibToken& rhs) const
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
				case Type_Int:			return getInt() == rhs.getInt();
				case Type_Float:		return getFloat() == rhs.getFloat();
				case Type_String:		return getString() == rhs.getString();
				case Type_IntArray:		return getIntArray() == rhs.getIntArray();
				case Type_FloatArray:	return getFloatArray() == rhs.getFloatArray();
				case Type_StringArray:	return getStringArray() == rhs.getStringArray();
				case Type_Request:		return getReq() == rhs.getReq();
				case Type_Ignore:		return true;
			}
			BOOST_REQUIRE(0 && "unrecognised type??");
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
		case Type_Ignore:		out << "IGNORE";			break;
	}
	return out;
}

// Compare the contents of a std::vector to a raw C array containing the same
// type.
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

// Compare a token containing an array to a raw C array.
//
// assert if the token doesn't contain an array.
bool tokenVoidCompareEqual(const CqMockRibToken& lhs, const void* rhs)
{
	switch(lhs.type())
	{
		case Type_IntArray:		return arrayEqual(lhs.getIntArray(), rhs);
		case Type_FloatArray:	return arrayEqual(lhs.getFloatArray(), rhs);
		case Type_StringArray:	return arrayEqual(lhs.getStringArray(), rhs);
		default:
			BOOST_REQUIRE(0 && "CqMockRibToken void* compare not implemented for type");
	}
	return false;
}


//------------------------------------------------------------------------------

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

		void discardIgnoreToks()
		{
			// discard any empty tokens.
			while(m_tokenPos < static_cast<TqInt>(m_params.size())
					&& m_params[m_tokenPos].type() == Type_Ignore)
			{
				++m_tokenPos;
			}
		}
		void checkNextType(EqType nextTokType)
		{
			discardIgnoreToks();
			if(m_params[m_tokenPos].type() != nextTokType)
				AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
						"missmatched token type " << nextTokType << " requested");
		}
	public:
		// Helper class to insert parameters into the parser.
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

		virtual void pushInput(std::istream& inStream, const std::string& name,
				const TqCommentCallback& callback) {}
		virtual void popInput() {}
		virtual SqRibPos streamPos()
		{
			return SqRibPos(0, m_tokenPos, "test_token_stream");
		}

		virtual TqInt getInt()
		{
			checkNextType(Type_Int);
			return m_params[m_tokenPos++].getInt();
		}
		virtual TqFloat getFloat()
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
		virtual const TqFloatArray& getFloatArray(TqInt length = -1)
		{
			discardIgnoreToks();
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
				checkNextType(Type_FloatArray);
				const TqFloatArray& array = m_params[m_tokenPos++].getFloatArray();
				if(length >= 0 && static_cast<TqInt>(array.size()) != length)
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
			while(m_tokenPos < static_cast<TqInt>(m_params.size())
					&& m_params[m_tokenPos].type() != Type_Request)
			{
				checkNextType(Type_String);
				paramHandler.readParameter(getString(), *this);
			}
		}

		virtual EqRibToken peekNextType()
		{
			if(m_tokenPos >= static_cast<TqInt>(m_params.size()))
				return Tok_RequestEnd;
			discardIgnoreToks();
			switch(m_params[m_tokenPos].type())
			{
				case Type_Int:			return Tok_Int;
				case Type_Float:		return Tok_Float;
				case Type_String:		return Tok_String;
				case Type_IntArray:
				case Type_FloatArray:
				case Type_StringArray:	return Tok_Array;
				case Type_Request:		return Tok_RequestEnd;
				case Type_Ignore:		break;
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
			BOOST_REQUIRE(id.type() == Type_Int || id.type() == Type_String);
			RtPointer p = reinterpret_cast<RtPointer>(++m_currHandle);
			m_handleMap.push_back(std::make_pair(id, p));
			return p;
		}
		const CqMockRibToken& lookup(RtPointer handle) const
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


// Fixture used in all tests, containing an instance of CqRibRequestHandler to
// be tested, along with instances of supporting classes.
//
// The constructor installs the instance of SqRequestHandlerFixture into the
// global variable g_fixture, so only one of these should exist at any time.
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
};

// global to enable Ri* function implementations to validate the function
// parameters against those which were input to the handler via the parser.
static SqRequestHandlerFixture* g_fixture = 0;

SqRequestHandlerFixture::SqRequestHandlerFixture()
	: handler(),
	parser(handler),
	hasBeenChecked(false)
{
	// Make sure that only one instance can exist at a time.
	BOOST_REQUIRE(!g_fixture);
	g_fixture = this;
}

SqRequestHandlerFixture::~SqRequestHandlerFixture()
{
	g_fixture = 0;
}


// Inserter for inserting a token stream into CqMockParser
class Insert
{
	private:
		CqMockParser& m_parser;
	public:
		Insert(CqMockParser& parser) : m_parser(parser) {}
		~Insert()
		{
			m_parser.parseNextRequest();
			BOOST_REQUIRE(g_fixture);
			BOOST_CHECK(g_fixture->hasBeenChecked);
			g_fixture->hasBeenChecked = false;
		}

		Insert& operator<<(const CqMockRibToken& tok)
		{
			m_parser.params().push_back(tok);
			return *this;
		}
};


// struct used to hold a param list for insertion into a CheckParams() sequence.
struct ParamList
{
	TqInt count;
	RtToken* tokens;
	RtPointer* values;
	ParamList(TqInt count, RtToken tokens[], RtPointer values[])
		: count(count), tokens(tokens), values(values)
	{}
};

// Checker class for checking parameters passed to an RI function against the
// initial values provided to CqMockParser.
class CheckParams
{
	private:
		TqTokVec::const_iterator m_currParam;

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

		CheckParams& operator<<(const CqMockRibToken& val)
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
			for(TqInt i = 0; i < pList.count; ++i)
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
//   SqRequestHandlerFixture f;
//   Insert(f.parser) << Req("SomeRequestName") << 1.0 << 1.0 << "asdf";
//
// f contains the mock parser and an instance of CqRibRequestHandler which is
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
// SqRequestHandlerFixture, so can access the parameters passed by Insert() and
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
	SqRequestHandlerFixture f;
	f.parser.params() += Req("version"), 3.03f;
	f.parser.parseNextRequest();
}


//--------------------------------------------------
AQSIS_RI_SHARE RtToken RiDeclare(RtString name, RtString declaration)
{
	CheckParams() << Req("Declare") << name << declaration;
	g_fixture->hasBeenChecked = true;
	return 0;
}
BOOST_AUTO_TEST_CASE(RiDeclare_handler_test)
{
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("Declare") << "asdf" << "uniform float";
	Insert(f.parser) << Req("Sphere") << 1.0f << -1.0f << 1.0f << 360.0f
						<< "asdf" << IqRibParser::TqFloatArray(1, 42.0f);
}


//--------------------------------------------------
AQSIS_RI_SHARE RtVoid RiDepthOfField(RtFloat fstop, RtFloat focallength, RtFloat focaldistance)
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
BOOST_AUTO_TEST_CASE(RiDepthOfField_three_args_test)
{
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("DepthOfField") << 1.0f << 42.0f << 42.5f;
}
BOOST_AUTO_TEST_CASE(RiDepthOfField_no_args_test)
{
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("DepthOfField");
}


//--------------------------------------------------
AQSIS_RI_SHARE RtVoid RiColorSamples(RtInt N, RtFloat nRGB[], RtFloat RGBn[])
{
	CheckParams() << Req("ColorSamples")
		<< IqRibParser::TqFloatArray(nRGB, nRGB + 3*N)
		<< IqRibParser::TqFloatArray(RGBn, RGBn + 3*N);
}
BOOST_AUTO_TEST_CASE(RiColorSamples_handler_test)
{
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("ColorSamples") << IqRibParser::TqFloatArray(12, 1.0f)
		<< IqRibParser::TqFloatArray(12, 1.0f);
}


//--------------------------------------------------
AQSIS_RI_SHARE RtLightHandle RiLightSourceV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
	CheckParams() << Req("LightSource") << name << IgnoreParam()
		<< ParamList(count, tokens, values);
	return g_fixture->handleManager.insertHandle(g_fixture->parser.currParams()[2]);
}
BOOST_AUTO_TEST_CASE(RiLightSource_integer_id_test)
{
	// Test integer light identifiers
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("LightSource") << "blahlight" << 10;
}
BOOST_AUTO_TEST_CASE(RiLightSource_string_id_test)
{
	// Test string light identifiers
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("LightSource") << "blahlight" << "stringName";
}


//--------------------------------------------------
AQSIS_RI_SHARE RtVoid RiIlluminate(RtLightHandle light, RtBoolean onoff)
{
	CheckParams()
		<< Req("Illuminate")
		<< g_fixture->handleManager.lookup(light)
		<< static_cast<TqInt>(onoff);
}
BOOST_AUTO_TEST_CASE(RiIlluminate_handler_test)
{
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("LightSource") << "blahlight" << 10;
	Insert(f.parser) << Req("LightSource") << "asdflight" << 11;
	Insert(f.parser) << Req("LightSource") << "qwerlight" << "handleName";
	Insert(f.parser) << Req("Illuminate")  << 11 << 1;
	Insert(f.parser) << Req("Illuminate")  << 10 << 0;
	Insert(f.parser) << Req("Illuminate")  << "handleName" << 0;
}
BOOST_AUTO_TEST_CASE(RiIlluminate_bad_int_handle_test)
{
	SqRequestHandlerFixture f;
	BOOST_CHECK_THROW(
		Insert(f.parser) << Req("Illuminate") << 10 << 1,
		XqParseError
	);
}
BOOST_AUTO_TEST_CASE(RiIlluminate_bad_string_handle_test)
{
	SqRequestHandlerFixture f;
	BOOST_CHECK_THROW(
		Insert(f.parser) << Req("Illuminate") << "asdf" << 1,
		XqParseError
	);
}


//--------------------------------------------------
AQSIS_RI_SHARE RtVoid RiBasis(RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep)
{
	BOOST_CHECK_EQUAL(::RiBSplineBasis, ubasis);
	TqFloat* vbStart = reinterpret_cast<TqFloat*>(vbasis);
	CheckParams() << Req("Basis")
		<< IgnoreParam() << ustep <<
		IqRibParser::TqFloatArray(vbStart, vbStart+16) << vstep;
}
BOOST_AUTO_TEST_CASE(RiBasis_handler_test)
{
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("Basis") << "b-spline"
		<< 1 << IqRibParser::TqFloatArray(16, 2.0f) << 42;
}


//--------------------------------------------------
AQSIS_RI_SHARE RtVoid RiSubdivisionMeshV(RtToken scheme, RtInt nfaces, RtInt nvertices[],
		RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[],
		RtInt intargs[], RtFloat floatargs[],
		RtInt count, RtToken tokens[], RtPointer values[])
{
	// total number of distinct vertices (number in "P" array)
	TqInt totVertices = 0;
	// total number of face vertices (size of vertices[])
	TqInt vertNum = 0;
	for(TqInt face = 0; face < nfaces; ++face)
	{
		for(TqInt i = 0; i < nvertices[face]; ++i)
		{
			totVertices = max(vertices[vertNum], totVertices);
			++vertNum;
		}
	}
	TqInt numIntArgs = 0;
	TqInt numFloatArgs = 0;
	for(TqInt tag = 0; tag < ntags; ++tag)
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

BOOST_AUTO_TEST_CASE(RiSubdivisionMesh_full_form_test)
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

	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("SubdivisionMesh")
		<< "catmull-clark" << nvertices << vertices << tags << nargs
		<< intargs << floatargs << "P" << P;
}

BOOST_AUTO_TEST_CASE(RiSubdivisionMesh_abbreviated_form_test)
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

	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("SubdivisionMesh")
		<< "catmull-clark" << nvertices << vertices
		<< IgnoreParam() << IgnoreParam() << IgnoreParam() << IgnoreParam() 
		<< "P" << P;
}


//--------------------------------------------------
AQSIS_RI_SHARE RtVoid RiHyperboloidV(RtPoint point1, RtPoint point2, RtFloat thetamax,
		RtInt count, RtToken tokens[], RtPointer values[])
{
	CheckParams() << Req("Hyperboloid") << point1[0] << point1[1] << point1[2]
		<< point2[0] << point2[1] << point2[2] << thetamax
		<< ParamList(count, tokens, values);
}
BOOST_AUTO_TEST_CASE(RiHyperboloidV_handler_test)
{
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("Hyperboloid") << 1.0f << 2.0f << 3.0f
		<< 4.0f << 5.0f << 6.0f << 360.0f;
}


//--------------------------------------------------
AQSIS_RI_SHARE RtVoid RiProcedural(RtPointer data, RtBound bound, RtProcSubdivFunc refineproc, RtProcFreeFunc freeproc)
{
	// All the standard procedurals should have RiProcFree
	BOOST_CHECK_EQUAL(freeproc, RiProcFree);

	// The following checking is specific to the ProcRunProgram procedural.
	BOOST_CHECK_EQUAL(refineproc, RiProcRunProgram);

	// The following checking is valid for ProcRunProgram and ProcDynamicLoad
	char** dataArray = reinterpret_cast<char**>(data);
	TqFloat* boundArray = reinterpret_cast<TqFloat*>(bound);
	CheckParams() << Req("Procedural") << IgnoreParam()
		<< IqRibParser::TqStringArray(dataArray, dataArray+2)
		<< IqRibParser::TqFloatArray(boundArray, boundArray+6);
}
BOOST_AUTO_TEST_CASE(RiProcedural_handler_test)
{
	SqRequestHandlerFixture f;
	IqRibParser::TqStringArray args; args += "progname", "some arg string";
	Insert(f.parser) << Req("Procedural") << "RunProgram"
		<< args << IqRibParser::TqFloatArray(6,1.0f);
}
BOOST_AUTO_TEST_CASE(RiProcedural_unknown_procedural_test)
{
	SqRequestHandlerFixture f;
	BOOST_CHECK_THROW(
		Insert(f.parser) << Req("Procedural") << "SomeNonexistantProcName"
			<< IqRibParser::TqStringArray(1,"asdf") << IqRibParser::TqFloatArray(6,1.0f),
		XqParseError
	);
}


//--------------------------------------------------
AQSIS_RI_SHARE RtObjectHandle RiObjectBegin()
{
	CheckParams() << Req("ObjectBegin");
	return g_fixture->handleManager.insertHandle(g_fixture->parser.currParams()[1]);
}
BOOST_AUTO_TEST_CASE(RiObjectBegin_integer_id_test)
{
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("ObjectBegin") << 42;
}
BOOST_AUTO_TEST_CASE(RiObjectBegin_string_id_test)
{
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("ObjectBegin") << "a_string_handle";
}
BOOST_AUTO_TEST_CASE(RiObjectBegin_bad_float_id_test)
{
	SqRequestHandlerFixture f;
	BOOST_CHECK_THROW(
		Insert(f.parser) << Req("ObjectBegin") << 42.5f,
		XqParseError
	);
}


//--------------------------------------------------
AQSIS_RI_SHARE RtVoid RiObjectInstance(RtObjectHandle handle)
{
	CheckParams() << Req("ObjectInstance")
		<< g_fixture->handleManager.lookup(handle);
}
BOOST_AUTO_TEST_CASE(RiObjectInstance_integer_id_lookup)
{
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("ObjectBegin") << 42;
	Insert(f.parser) << Req("ObjectBegin") << 2;
	Insert(f.parser) << Req("ObjectInstance") << 2;
	Insert(f.parser) << Req("ObjectInstance") << 42;
}
BOOST_AUTO_TEST_CASE(RiObjectInstance_string_id_lookup)
{
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("ObjectBegin") << "a_string_handle";
	Insert(f.parser) << Req("ObjectInstance") << "a_string_handle";
}
BOOST_AUTO_TEST_CASE(RiObjectInstance_undeclared_error_test)
{
	SqRequestHandlerFixture f;
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
AQSIS_RI_SHARE RtVoid RiSphereV(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax,
		RtInt count, RtToken tokens[], RtPointer values[])
{
	CheckParams() << Req("Sphere") << radius << zmin << zmax << thetamax
		<< ParamList(count, tokens, values);
}
BOOST_AUTO_TEST_CASE(RiSphereV_handler_test)
{
	// Test RiSphere; also test the handling of valid parameter lists.
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("Sphere") << 2.5f << -1.0f << 1.0f << 360.0f
		<< "uniform float blah" << IqRibParser::TqFloatArray(2, 42.25f)
		<< "Cs" << IqRibParser::TqFloatArray(4, 2.0f);
}


//--------------------------------------------------
AQSIS_RI_SHARE RtVoid RiColor(RtColor col)
{
	CheckParams() << Req("Color") << col[0] << col[1] << col[2];
}
BOOST_AUTO_TEST_CASE(RiColor_handler_test)
{
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("Color") << 1.0f << 2.0f << 3.0f;
}


//--------------------------------------------------
AQSIS_RI_SHARE RtVoid RiPixelFilter(RtFilterFunc function, RtFloat xwidth, RtFloat ywidth)
{
	BOOST_CHECK_EQUAL(function, RiSincFilter);
	CheckParams() << Req("PixelFilter") << IgnoreParam() << xwidth << ywidth;
}
BOOST_AUTO_TEST_CASE(RiPixelFilter_handler_test)
{
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("PixelFilter") << "sinc" << 7.0f << 7.5f;
}
BOOST_AUTO_TEST_CASE(RiPixelFilter_bad_filter_name)
{
	SqRequestHandlerFixture f;
	BOOST_CHECK_THROW(
		Insert(f.parser) << Req("PixelFilter") << "nonexistent" << 7.0f << 7.5f,
		XqParseError
	);
}


//--------------------------------------------------
AQSIS_RI_SHARE RtVoid RiErrorHandler(RtErrorFunc handler)
{
	BOOST_CHECK_EQUAL(handler, RiErrorAbort);
	CheckParams() << Req("ErrorHandler");
}
BOOST_AUTO_TEST_CASE(RiErrorFunc_handler_test)
{
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("ErrorHandler") << "abort";
}
BOOST_AUTO_TEST_CASE(RiErrorFunc_bad_error_handler_name)
{
	SqRequestHandlerFixture f;
	BOOST_CHECK_THROW(
		Insert(f.parser) << Req("ErrorHandler") << "bogus",
		XqParseError
	);
}


//--------------------------------------------------
AQSIS_RI_SHARE RtVoid RiTransform(RtMatrix transform)
{
	TqFloat* trans = reinterpret_cast<TqFloat*>(transform);
	CheckParams() << Req("Transform")
		<< IqRibParser::TqFloatArray(trans, trans+16);
}
BOOST_AUTO_TEST_CASE(RiTransform_handler_test)
{
	TqFloat trans[16] = {
		1,2,3,4,
		5,6,7,8,
		9,-1,-2,-3,
		-4,-5,-6,-7
	};

	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("Transform")
		<< IqRibParser::TqFloatArray(trans, trans+16);
}

AQSIS_RI_SHARE RtVoid RiPointsV(RtInt npoints, RtInt count, RtToken tokens[], RtPointer values[])
{
	// This check will only work for the specific case below
	BOOST_CHECK_EQUAL(npoints, 4);
	CheckParams() << Req("Points") << ParamList(count, tokens, values);
}
BOOST_AUTO_TEST_CASE(RiPoints_handler_test)
{
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("Points")
		<< "Cs" << IqRibParser::TqFloatArray(12, 0.5f)
		<< "P" << IqRibParser::TqFloatArray(12, 1.5f);
}
BOOST_AUTO_TEST_CASE(RiPoints_missing_P_variable)
{
	SqRequestHandlerFixture f;
	BOOST_CHECK_THROW(
		Insert(f.parser) << Req("Points")
			<< "Cs" << IqRibParser::TqFloatArray(12, 0.5f),
		XqParseError
	);
}


//--------------------------------------------------
AQSIS_RI_SHARE RtVoid RiMotionBeginV(RtInt N, RtFloat times[])
{
	CheckParams() << Req("MotionBegin")
		<< IqRibParser::TqFloatArray(times, times+N);
}
BOOST_AUTO_TEST_CASE(RiMotionBegin_handler_test)
{
	SqRequestHandlerFixture f;
	IqRibParser::TqFloatArray times;
	times += 0, 1, 2.5, 3, 4;
	Insert(f.parser) << Req("MotionBegin") << times;
}


//--------------------------------------------------
AQSIS_RI_SHARE RtVoid RiMakeOcclusionV(RtInt npics, RtString picfiles[], RtString shadowfile,
		RtInt count, RtToken tokens[], RtPointer values[])
{
	CheckParams() << Req("MakeOcclusion")
		<< IqRibParser::TqStringArray(picfiles, picfiles+npics)
		<< shadowfile << ParamList(count, tokens, values);
}
BOOST_AUTO_TEST_CASE(RiMakeOcclusion_handler_test)
{
	IqRibParser::TqStringArray picFiles;
	picFiles += "pic1", "pic2", "pic3", "some_pic4", "asdf5", "Occlmap6.om";
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("MakeOcclusion") << picFiles << "ambient.map";
}


//------------------------------------------------------------------------------
// Test for parameter list handling, using RiOption as a proxy.
//
AQSIS_RI_SHARE RtVoid RiOptionV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
{
	CheckParams() << Req("Option") << name << ParamList(count, tokens, values);
	// Check that the parameter list token count is correct.  Note that the
	// formula used below assumes there's only one Req in the token stream.
	BOOST_CHECK_EQUAL(2*count, static_cast<TqInt>(g_fixture->parser.params().size()) - 2);
}

BOOST_AUTO_TEST_CASE(paramlist_int_array_value_test)
{
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("Option") << "some_option_name"
		<< "uniform int asdf" << IqRibParser::TqIntArray(10, 42);
}
BOOST_AUTO_TEST_CASE(paramlist_float_array_value_test)
{
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("Option") << "some_option_name"
		<< "uniform float asdf" << IqRibParser::TqFloatArray(10, 2.5f);
}
BOOST_AUTO_TEST_CASE(paramlist_string_array_value_test)
{
	SqRequestHandlerFixture f;
	Insert(f.parser) << Req("Option") << "some_option_name"
		<< "uniform string asdf" << IqRibParser::TqStringArray(10, "asdf_value");
}
BOOST_AUTO_TEST_CASE(extended_paramlist_test)
{
	SqRequestHandlerFixture f;
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
	SqRequestHandlerFixture f;
	BOOST_CHECK_THROW(
		Insert(f.parser) << Req("Option") << "some_option_name"
			<< "asdf" << IqRibParser::TqFloatArray(1, 42.25f),
		XqParseError
	);
}
BOOST_AUTO_TEST_CASE(invalid_paramlist_invalid_primvar)
{
	// Check that invalid primvar strings throw.
	SqRequestHandlerFixture f;
	BOOST_CHECK_THROW(
		Insert(f.parser) << Req("Option") << "some_option_name"
			<< "] bad_token" << IqRibParser::TqFloatArray(1, 42.25f),
		XqParseError
	);
}
BOOST_AUTO_TEST_CASE(invalid_paramlist_missing_token)
{
	// check that a missing token throws.
	SqRequestHandlerFixture f;
	BOOST_CHECK_THROW(
		Insert(f.parser) << Req("Option") << "some_option_name"
			<< "P" << IqRibParser::TqFloatArray(4, 42.25f)
			<< IqRibParser::TqIntArray(1, 42),
		XqParseError
	);
}


//==============================================================================
// Definitions of RI symbols

// bases
AQSIS_RI_SHARE RtBasis RiBezierBasis =
{
	{ -1,  3, -3, 1},
	{  3, -6,  3, 0},
	{ -3,  3,  0, 0},
	{  1,  0,  0, 0}
};
AQSIS_RI_SHARE RtBasis RiBSplineBasis =
{
	{ -1.0 / 6,  3.0 / 6, -3.0 / 6, 1.0 / 6},
	{  3.0 / 6, -6.0 / 6,  3.0 / 6, 0.0 / 6},
	{ -3.0 / 6,  0.0 / 6,  3.0 / 6, 0.0 / 6},
	{  1.0 / 6,  4.0 / 6,  1.0 / 6, 0.0 / 6}
};
AQSIS_RI_SHARE RtBasis RiCatmullRomBasis =
{
	{ -1.0 / 2,  3.0 / 2, -3.0 / 2,  1.0 / 2},
	{  2.0 / 2, -5.0 / 2,  4.0 / 2, -1.0 / 2},
	{ -1.0 / 2,  0.0 / 2,  1.0 / 2,  0.0 / 2},
	{  0.0 / 2,  2.0 / 2,  0.0 / 2,  0.0 / 2}
};
AQSIS_RI_SHARE RtBasis RiHermiteBasis =
{
	{  2,  1, -2,  1},
	{ -3, -2,  3, -1},
	{  0,  1,  0,  0},
	{  1,  0,  0,  0}
};
AQSIS_RI_SHARE RtBasis RiPowerBasis =
{
	{ 1, 0, 0, 0},
	{ 0, 1, 0, 0},
	{ 0, 0, 1, 0},
	{ 0, 0, 0, 1}
};


// Empty implementations of all required Ri* functions.
// Autogenerated via XSLT, with those functions actually used above commented out.

//RtToken RiDeclare(RtString name, RtString declaration) {return 0;}
AQSIS_RI_SHARE RtVoid RiBegin(RtToken name) {}
AQSIS_RI_SHARE RtVoid RiEnd() {}
AQSIS_RI_SHARE RtContextHandle RiGetContext() {return 0;}
AQSIS_RI_SHARE RtVoid RiContext(RtContextHandle handle) {}
AQSIS_RI_SHARE RtVoid RiFrameBegin(RtInt number) {}
AQSIS_RI_SHARE RtVoid RiFrameEnd() {}
AQSIS_RI_SHARE RtVoid RiWorldBegin() {}
AQSIS_RI_SHARE RtVoid RiWorldEnd() {}
AQSIS_RI_SHARE RtVoid RiIfBegin(RtString condition) {}
AQSIS_RI_SHARE RtVoid RiElseIf(RtString condition) {}
AQSIS_RI_SHARE RtVoid RiElse() {}
AQSIS_RI_SHARE RtVoid RiIfEnd() {}
AQSIS_RI_SHARE RtVoid RiFormat(RtInt xresolution, RtInt yresolution, RtFloat pixelaspectratio) {}
AQSIS_RI_SHARE RtVoid RiFrameAspectRatio(RtFloat frameratio) {}
AQSIS_RI_SHARE RtVoid RiScreenWindow(RtFloat left, RtFloat right, RtFloat bottom, RtFloat top) {}
AQSIS_RI_SHARE RtVoid RiCropWindow(RtFloat xmin, RtFloat xmax, RtFloat ymin, RtFloat ymax) {}
AQSIS_RI_SHARE RtVoid RiProjectionV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiClipping(RtFloat cnear, RtFloat cfar) {}
AQSIS_RI_SHARE RtVoid RiClippingPlane(RtFloat x, RtFloat y, RtFloat z, RtFloat nx, RtFloat ny, RtFloat nz) {}
//RtVoid RiDepthOfField(RtFloat fstop, RtFloat focallength, RtFloat focaldistance) {}
AQSIS_RI_SHARE RtVoid RiShutter(RtFloat opentime, RtFloat closetime) {}
AQSIS_RI_SHARE RtVoid RiPixelVariance(RtFloat variance) {}
AQSIS_RI_SHARE RtVoid RiPixelSamples(RtFloat xsamples, RtFloat ysamples) {}
//RtVoid RiPixelFilter(RtFilterFunc function, RtFloat xwidth, RtFloat ywidth) {}
AQSIS_RI_SHARE RtVoid RiExposure(RtFloat gain, RtFloat gamma) {}
AQSIS_RI_SHARE RtVoid RiImagerV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiQuantize(RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ditheramplitude) {}
AQSIS_RI_SHARE RtVoid RiDisplayV(RtToken name, RtToken type, RtToken mode, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtFloat RiGaussianFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) {return 0;}
AQSIS_RI_SHARE RtFloat RiBoxFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) {return 0;}
AQSIS_RI_SHARE RtFloat RiMitchellFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) {return 0;}
AQSIS_RI_SHARE RtFloat RiTriangleFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) {return 0;}
AQSIS_RI_SHARE RtFloat RiCatmullRomFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) {return 0;}
AQSIS_RI_SHARE RtFloat RiSincFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) {return 0;}
AQSIS_RI_SHARE RtFloat RiDiskFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) {return 0;}
AQSIS_RI_SHARE RtFloat RiBesselFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) {return 0;}
AQSIS_RI_SHARE RtVoid RiHiderV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
//RtVoid RiColorSamples(RtInt N, RtFloat nRGB[], RtFloat RGBn[]) {}
AQSIS_RI_SHARE RtVoid RiRelativeDetail(RtFloat relativedetail) {}
//RtVoid RiOptionV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiAttributeBegin() {}
AQSIS_RI_SHARE RtVoid RiAttributeEnd() {}
//RtVoid RiColor(RtColor Cq) {}
AQSIS_RI_SHARE RtVoid RiOpacity(RtColor Os) {}
AQSIS_RI_SHARE RtVoid RiTextureCoordinates(RtFloat s1, RtFloat t1, RtFloat s2, RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4, RtFloat t4) {}
//RtLightHandle RiLightSourceV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {return 0;}
AQSIS_RI_SHARE RtLightHandle RiAreaLightSourceV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {return 0;}
//RtVoid RiIlluminate(RtLightHandle light, RtBoolean onoff) {}
AQSIS_RI_SHARE RtVoid RiSurfaceV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiDeformationV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiDisplacementV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiAtmosphereV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiInteriorV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiExteriorV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiShaderLayerV(RtToken type, RtToken name, RtToken layername, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiConnectShaderLayers(RtToken type, RtToken layer1, RtToken variable1, RtToken layer2, RtToken variable2) {}
AQSIS_RI_SHARE RtVoid RiShadingRate(RtFloat size) {}
AQSIS_RI_SHARE RtVoid RiShadingInterpolation(RtToken type) {}
AQSIS_RI_SHARE RtVoid RiMatte(RtBoolean onoff) {}
AQSIS_RI_SHARE RtVoid RiBound(RtBound bound) {}
AQSIS_RI_SHARE RtVoid RiDetail(RtBound bound) {}
AQSIS_RI_SHARE RtVoid RiDetailRange(RtFloat offlow, RtFloat onlow, RtFloat onhigh, RtFloat offhigh) {}
AQSIS_RI_SHARE RtVoid RiGeometricApproximation(RtToken type, RtFloat value) {}
AQSIS_RI_SHARE RtVoid RiOrientation(RtToken orientation) {}
AQSIS_RI_SHARE RtVoid RiReverseOrientation() {}
AQSIS_RI_SHARE RtVoid RiSides(RtInt nsides) {}
AQSIS_RI_SHARE RtVoid RiIdentity() {}
//RtVoid RiTransform(RtMatrix transform) {}
AQSIS_RI_SHARE RtVoid RiConcatTransform(RtMatrix transform) {}
AQSIS_RI_SHARE RtVoid RiPerspective(RtFloat fov) {}
AQSIS_RI_SHARE RtVoid RiTranslate(RtFloat dx, RtFloat dy, RtFloat dz) {}
AQSIS_RI_SHARE RtVoid RiRotate(RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz) {}
AQSIS_RI_SHARE RtVoid RiScale(RtFloat sx, RtFloat sy, RtFloat sz) {}
AQSIS_RI_SHARE RtVoid RiSkew(RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1, RtFloat dx2, RtFloat dy2, RtFloat dz2) {}
AQSIS_RI_SHARE RtVoid RiCoordinateSystem(RtToken space) {}
AQSIS_RI_SHARE RtVoid RiCoordSysTransform(RtToken space) {}
AQSIS_RI_SHARE RtPoint* RiTransformPoints(RtToken fromspace, RtToken tospace, RtInt npoints, RtPoint points[]) {return 0;}
AQSIS_RI_SHARE RtVoid RiTransformBegin() {}
AQSIS_RI_SHARE RtVoid RiTransformEnd() {}
AQSIS_RI_SHARE RtVoid RiResourceV(RtToken handle, RtToken type, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiResourceBegin() {}
AQSIS_RI_SHARE RtVoid RiResourceEnd() {}
AQSIS_RI_SHARE RtVoid RiAttributeV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiPolygonV(RtInt nvertices, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiGeneralPolygonV(RtInt nloops, RtInt nverts[], RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiPointsPolygonsV(RtInt npolys, RtInt nverts[], RtInt verts[], RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiPointsGeneralPolygonsV(RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], RtInt count, RtToken tokens[], RtPointer values[]) {}
//RtVoid RiBasis(RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep) {}
AQSIS_RI_SHARE RtVoid RiPatchV(RtToken type, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiPatchMeshV(RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiNuPatchV(RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiTrimCurve(RtInt nloops, RtInt ncurves[], RtInt order[], RtFloat knot[], RtFloat min[], RtFloat max[], RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[]) {}
//RtVoid RiSubdivisionMeshV(RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], RtInt count, RtToken tokens[], RtPointer values[]) {}
//RtVoid RiSphereV(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiConeV(RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiCylinderV(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) {}
//RtVoid RiHyperboloidV(RtPoint point1, RtPoint point2, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiParaboloidV(RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiDiskV(RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiTorusV(RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) {}
//RtVoid RiPointsV(RtInt npoints, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiCurvesV(RtToken type, RtInt ncurves, RtInt nvertices[], RtToken wrap, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiBlobbyV(RtInt nleaf, RtInt ncode, RtInt code[], RtInt nflt, RtFloat flt[], RtInt nstr, RtToken str[], RtInt count, RtToken tokens[], RtPointer values[]) {}
//RtVoid RiProcedural(RtPointer data, RtBound bound, RtProcSubdivFunc refineproc, RtProcFreeFunc freeproc) {}
AQSIS_RI_SHARE RtVoid RiProcFree(RtPointer data) {}
AQSIS_RI_SHARE RtVoid RiProcDelayedReadArchive(RtPointer data, RtFloat detail) {}
AQSIS_RI_SHARE RtVoid RiProcRunProgram(RtPointer data, RtFloat detail) {}
AQSIS_RI_SHARE RtVoid RiProcDynamicLoad(RtPointer data, RtFloat detail) {}
AQSIS_RI_SHARE RtVoid RiGeometryV(RtToken type, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiSolidBegin(RtToken type) {}
AQSIS_RI_SHARE RtVoid RiSolidEnd() {}
//RtObjectHandle RiObjectBegin() {return 0;}
AQSIS_RI_SHARE RtVoid RiObjectEnd() {}
//RtVoid RiObjectInstance(RtObjectHandle handle) {}
AQSIS_RI_SHARE RtVoid RiMotionBegin(RtInt N,  ...) {}
//RtVoid RiMotionBeginV(RtInt N, RtFloat times[]) {}
AQSIS_RI_SHARE RtVoid RiMotionEnd() {}
AQSIS_RI_SHARE RtVoid RiMakeTextureV(RtString imagefile, RtString texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiMakeBumpV(RtString imagefile, RtString bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiMakeLatLongEnvironmentV(RtString imagefile, RtString reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiMakeCubeFaceEnvironmentV(RtString px, RtString nx, RtString py, RtString ny, RtString pz, RtString nz, RtString reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[]) {}
AQSIS_RI_SHARE RtVoid RiMakeShadowV(RtString picfile, RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[]) {}
//RtVoid RiMakeOcclusionV(RtInt npics, RtString picfiles[], RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[]) {}
//RtVoid RiErrorHandler(RtErrorFunc handler) {}
AQSIS_RI_SHARE RtVoid RiErrorIgnore(RtInt code, RtInt severity, RtString message) {}
AQSIS_RI_SHARE RtVoid RiErrorPrint(RtInt code, RtInt severity, RtString message) {}
AQSIS_RI_SHARE RtVoid RiErrorAbort(RtInt code, RtInt severity, RtString message) {}
AQSIS_RI_SHARE RtVoid RiArchiveRecord(RtToken type, char * format,  ...) {}
AQSIS_RI_SHARE RtVoid RiReadArchiveV(RtToken name, RtArchiveCallback callback, RtInt count, RtToken tokens[], RtPointer values[]) {}
