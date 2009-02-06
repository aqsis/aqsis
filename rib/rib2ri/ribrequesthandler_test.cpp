#include "ribrequesthandler.h"

#include <sstream>

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <boost/any.hpp>
#include <boost/assign/std/vector.hpp>

#include "ri.h"
#include "iribparser.h"
#include "smartptr.h"

using namespace boost::assign; // necessary for container initialisation operators.

using namespace Aqsis;


//------------------------------------------------------------------------------
typedef std::vector<boost::any> TqAnyVec;

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

template<typename T>
bool arrayEqual(const std::vector<T>& v1, RtPointer v2)
{
	const T* v2T = reinterpret_cast<const T*>(v2);
	for(TqInt i = 0, end = v1.size(); i < end; ++i)
	{
		if(v1[i] != v2T[i])
			return false;
	}
	return true;
}

//------------------------------------------------------------------------------
// Extensions to the boost::any type, allowing for restricted comparison and
// stream insertion operations.  They're contained in this namespace for safety.
namespace boost_any_extensions {

// Helper struct to disallow implicit anything -> boost::any conversions for
// functions below.
struct SqAnyWrapper
{
	const boost::any& value;
	SqAnyWrapper(const boost::any& value)
		: value(value)
	{}
};

/* Insert a boost::any into a stream.
 *
 * It's dangerous to allow boost::any to be taken directly as an argument here,
 * since it has an implicit conversion operator from just about anything.
 * Instead we exploit the fact that there can be only one implicit conversion
 * in a chain to disallow implicit conversions using the SqAnyWrapper struct.
 */
std::ostream& operator<<(std::ostream& out, const SqAnyWrapper& wrappedAny)
{
	using ::operator<<; // for std::vector<T> inserter above.
	const boost::any& a = wrappedAny.value;
	// This if/else stuff is very ugly but the elegant solution would require
	// writing a modified boost::any which is more work.
	if(a.type() == typeid(int))
		out << boost::any_cast<int>(a);
	else if(a.type() == typeid(float))
		out << boost::any_cast<float>(a);
	else if(a.type() == typeid(const char*))
		out << "\"" << boost::any_cast<const char*>(a) << "\"";
	else if(a.type() == typeid(std::string))
		out << "\"" << boost::any_cast<std::string>(a) << "\"";
	else if(a.type() == typeid(std::vector<float>))
		out << boost::any_cast<const std::vector<float>&>(a);
	else if(a.type() == typeid(std::vector<std::string>))
		out << boost::any_cast<const std::vector<std::string>&>(a);
	else
		throw std::runtime_error(std::string("Unknown boost::any type: ")
				+ a.type().name());
	return out;
}

/* Compare two boost::any for equality.
 *
 * As for operator<<(), taking boost::any directly would be dangerous here
 * because of implicit conversions which we disallow with SqAnyWrapper.
 */
bool operator==(const SqAnyWrapper& wrappedAnyA, const SqAnyWrapper& wrappedAnyB)
{
	const boost::any& a = wrappedAnyA.value;
	const boost::any& b = wrappedAnyB.value;
	if(a.type() != b.type())
		return false;
	if(a.type() == typeid(int))
		return boost::any_cast<int>(a) == boost::any_cast<int>(b);
	else if(a.type() == typeid(float))
		return boost::any_cast<float>(a) == boost::any_cast<float>(b);
	else if(a.type() == typeid(const char*))
		return std::string(boost::any_cast<const char*>(a))
			== boost::any_cast<const char*>(b);
	if(a.type() == typeid(IqRibParser::TqIntArray))
		return boost::any_cast<IqRibParser::TqIntArray>(a)
			== boost::any_cast<IqRibParser::TqIntArray>(b);
	if(a.type() == typeid(IqRibParser::TqFloatArray))
		return boost::any_cast<IqRibParser::TqFloatArray>(a)
			== boost::any_cast<IqRibParser::TqFloatArray>(b);
	if(a.type() == typeid(IqRibParser::TqStringArray))
		return boost::any_cast<IqRibParser::TqStringArray>(a)
			== boost::any_cast<IqRibParser::TqStringArray>(b);
	else
		throw std::runtime_error(std::string("Unknown boost::any type: ")
				+ a.type().name());
}

/* Compare a boost::any and an RtPointer for equality.
 *
 * This is used for comparing an input value to a RI parameter list (stored as
 * a boost::any) to the value which comes out (stored as a void*).  Type
 * deduction is via boost::any::type().
 */
bool operator==(const SqAnyWrapper& wrappedAnyA, RtPointer b)
{
	const boost::any& a = wrappedAnyA.value;
	if(a.type() == typeid(int))
		return boost::any_cast<int>(a) == *reinterpret_cast<int*>(b);
	else if(a.type() == typeid(float))
		return boost::any_cast<float>(a) == *reinterpret_cast<float*>(b);
	else if(a.type() == typeid(const char*))
		return boost::any_cast<const char*>(a) == *reinterpret_cast<RtString*>(b);
	if(a.type() == typeid(IqRibParser::TqIntArray))
		return arrayEqual(boost::any_cast<IqRibParser::TqIntArray>(a), b);
	if(a.type() == typeid(IqRibParser::TqFloatArray))
		return arrayEqual(boost::any_cast<IqRibParser::TqFloatArray>(a), b);
	if(a.type() == typeid(IqRibParser::TqStringArray))
		return arrayEqual(boost::any_cast<IqRibParser::TqStringArray>(a), b);
	else
		throw std::runtime_error(std::string("Unknown boost::any type: ")
				+ a.type().name());
}

} // namespace boost_any_extensions

namespace boost {
	// Add boost_any_extensions operators to the boost namespace so that they
	// can be found inside BOOST_CHECK_* functions.
	using boost_any_extensions::operator<<;
	using boost_any_extensions::operator==;
}


//------------------------------------------------------------------------------
// Struct holding RI function input parameters.
struct SqRiInputParams
{
	std::string requestName;
	TqAnyVec params;
	bool hasBeenChecked;

	// Create an empty input and set the global g_input variable to this.
	SqRiInputParams();
	// Set the global g_input variable to 0.
	~SqRiInputParams();
};

// global SqRiInputParams to enable Ri* function implementations to validate the
// function parameters against those which were input to the handler.
static SqRiInputParams* g_input = 0;

SqRiInputParams::SqRiInputParams()
	: requestName(),
	params(),
	hasBeenChecked(false)
{
	g_input = this;
}

SqRiInputParams::~SqRiInputParams()
{
	g_input = 0;
}

// Insert parameters into a stream in RIB format.
std::ostream& operator<<(std::ostream& out, const SqRiInputParams& paramInput)
{
	out << paramInput.requestName << " ";
	// Positional params
	for(int i = 0; i < static_cast<TqInt>(paramInput.params.size()); ++i)
		out << paramInput.params[i] << " ";
	return out;
}


//------------------------------------------------------------------------------
// Test fixtures, mock objects and checking functions.

// Mock implementation of the RIB parser interface for testing purposes.
//
// This "parser" takes a SqRiInputParams structure describing the input which
// should be provided to the request handler object.  When data is requested,
// the mock parser just uses boost::any_cast to cast the current input element
// to the requested type; an exception is thrown if the types don't match.
class CqMockParser : public IqRibParser
{
	private:
		const SqRiInputParams& m_params;
		CqRibRequestHandler& m_handler;
		int m_tokenPos;
		std::vector<TqFloatArray> m_floatVecStorage;
	public:
		CqMockParser(const SqRiInputParams& params, CqRibRequestHandler& handler)
			: m_params(params),
			m_handler(handler),
			m_tokenPos(0)
		{ }

		virtual bool parseNextRequest()
		{
			m_handler.handleRequest(m_params.requestName, *this);
			return false;
		}

		virtual void pushInput(std::istream& inStream, const std::string& name) {}
		virtual void popInput() {}

#		define TRY_GUARD try{
#		define CATCH_GUARD(funcName) } catch(boost::bad_any_cast&) { throw std::runtime_error("bad cast in get" funcName "()"); };
		virtual TqInt getInt()
		{
			TRY_GUARD
			return boost::any_cast<int>(m_params.params[m_tokenPos++]);
			CATCH_GUARD("Int")
		}
		virtual TqFloat getFloat()
		{
			TRY_GUARD
			return boost::any_cast<float>(m_params.params[m_tokenPos++]);
			CATCH_GUARD("Float")
		}
		virtual std::string getString()
		{
			TRY_GUARD
			return boost::any_cast<const char*>(m_params.params[m_tokenPos++]);
			CATCH_GUARD("String")
		}

		virtual const TqIntArray& getIntArray()
		{
			TRY_GUARD
			return boost::any_cast<const TqIntArray&>(m_params.params[m_tokenPos++]);
			CATCH_GUARD("IntArray")
		}
		virtual const TqFloatArray& getFloatArray(TqInt length = -1)
		{
			TRY_GUARD
			if(length >= 0 && m_params.params[m_tokenPos].type() == typeid(float))
			{
				m_floatVecStorage.push_back(TqFloatArray(length,0));
				// read in individual floats for the array
				TqFloatArray& array = m_floatVecStorage.back();
				for(TqInt i = 0; i < length; ++i)
					array[i] = boost::any_cast<float>(m_params.params[m_tokenPos++]);
				return array;
			}
			else
			{
				const TqFloatArray& array = 
					boost::any_cast<const TqFloatArray&>(m_params.params[m_tokenPos++]);
				if(length >= 0 && static_cast<TqInt>(array.size()) != length)
					throw std::runtime_error("Bad array length detected.");
				return array;
			}
			CATCH_GUARD("FloatArray")
		}
		virtual const TqStringArray& getStringArray()
		{
			TRY_GUARD
			return boost::any_cast<const TqStringArray&>(m_params.params[m_tokenPos++]);
			CATCH_GUARD("StringArray")
		}
#		undef TRY_GUARD
#		undef CATCH_GUARD

		virtual void getParamList(IqRibParamListHandler& paramHandler)
		{
			while(m_tokenPos < static_cast<TqInt>(m_params.params.size()))
				paramHandler.readParameter(getString(), *this);
		}

		virtual EqRibToken peekNextType()
		{
			if(m_tokenPos >= static_cast<TqInt>(m_params.params.size()))
				return Tok_RequestEnd;
			const std::type_info& type = m_params.params[m_tokenPos].type();
			if(type == typeid(int))
				return Tok_Int;
			if(type == typeid(float))
				return Tok_Float;
			if(type == typeid(const char*))
				return Tok_String;
			if(type == typeid(TqIntArray) || type == typeid(TqFloatArray)
					|| type == typeid(TqStringArray))
				return Tok_Array;
			throw std::runtime_error(
					std::string("peekNextType - type unknown: ") + type.name());
		}

		virtual const TqBasis* getBasis(const IqStringToBasis& stringToBasis)
		{
			return 0;
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


struct RequestHandlerFixture
{
	CqRibRequestHandler handler;
	CqMockParser parser;

	RequestHandlerFixture(const SqRiInputParams& params)
		: handler(),
		parser(params, handler)
	{ }
};


// Check equality of paramters with the input parameters held in the g_input
// global struct.
void checkParams(const std::string& name, const TqAnyVec& positionalParams,
		RtInt count, RtToken tokens[], RtPointer values[])
{
	BOOST_REQUIRE(g_input != 0);

	BOOST_CHECK_EQUAL(g_input->requestName, name);

	TqInt numParams = g_input->params.size();
	TqInt numPosParams = positionalParams.size();
	BOOST_REQUIRE(numParams >= numPosParams + count);
	// Check that all positional parameters match up.
	for(TqInt i = 0; i < numPosParams; ++i)
		BOOST_CHECK_EQUAL(g_input->params[i], positionalParams[i]);

	// Check that all parameter-list parameters match up.
	for(TqInt i = 0; i < count; ++i)
	{
		BOOST_CHECK_EQUAL(std::string(boost::any_cast<const char*>(
					g_input->params[numPosParams + 2*i])), tokens[i]);
		BOOST_CHECK_EQUAL(g_input->params[numPosParams + 2*i + 1], values[i]);
	}

	g_input->hasBeenChecked = true;
}



//==============================================================================
// Test cases

RtVoid RiSphereV(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax,
		RtInt count, RtToken tokens[], RtPointer values[])
{
	TqAnyVec posParams; posParams += radius, zmin, zmax, thetamax;
	checkParams("Sphere", posParams, count, tokens, values);
}

BOOST_AUTO_TEST_CASE(RiSphereV_handler_test)
{
	SqRiInputParams input;
	input.requestName = "Sphere";
	input.params += 2.5f, -1.0f, 1.0f, 360.0f, "uniform float blah", IqRibParser::TqFloatArray(2, 42.25f);
	// std::cerr << "Checking --- " << input << "\n";
	RequestHandlerFixture f(input);
	f.parser.parseNextRequest();
	BOOST_CHECK(input.hasBeenChecked);
}


RtVoid RiHyperboloidV(RtPoint point1, RtPoint point2, RtFloat thetamax,
		RtInt count, RtToken tokens[], RtPointer values[])
{
	TqAnyVec posParams; posParams += point1[0], point1[1], point1[2],
		point2[0], point2[1], point2[2], thetamax;
	checkParams("Hyperboloid", posParams, count, tokens, values);
}
BOOST_AUTO_TEST_CASE(RiHyperboloidV_handler_test)
{
	SqRiInputParams input;
	input.requestName = "Hyperboloid";
	input.params += 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 360.0f;
//	std::cerr << "Checking --- " << input << "\n";
	RequestHandlerFixture f(input);
	f.parser.parseNextRequest();
	BOOST_CHECK(input.hasBeenChecked);
}



//==============================================================================
// Definitions for Ri*Basis symbols

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


//------------------------------------------------------------------------------
// Empty implementations of all required Ri* functions.
// Autogenerated via XSLT.

RtToken RiDeclare(RtString name, RtString declaration) {return 0;}
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
RtVoid RiDepthOfField(RtFloat fstop, RtFloat focallength, RtFloat focaldistance) {}
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
RtVoid RiColorSamples(RtInt N, RtFloat nRGB[], RtFloat RGBn[]) {}
RtVoid RiRelativeDetail(RtFloat relativedetail) {}
RtVoid RiOptionV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiAttributeBegin() {}
RtVoid RiAttributeEnd() {}
RtVoid RiColor(RtColor Cq) {}
RtVoid RiOpacity(RtColor Os) {}
RtVoid RiTextureCoordinates(RtFloat s1, RtFloat t1, RtFloat s2, RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4, RtFloat t4) {}
RtLightHandle RiLightSourceV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {return 0;}
RtLightHandle RiAreaLightSourceV(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) {return 0;}
RtVoid RiIlluminate(RtLightHandle light, RtBoolean onoff) {}
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
RtVoid RiBasis(RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep) {}
RtVoid RiPatchV(RtToken type, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiPatchMeshV(RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiNuPatchV(RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiTrimCurve(RtInt nloops, RtInt ncurves[], RtInt order[], RtFloat knot[], RtFloat min[], RtFloat max[], RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[]) {}
RtVoid RiSubdivisionMeshV(RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], RtInt count, RtToken tokens[], RtPointer values[]) {}
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
RtVoid RiProcedural(RtPointer data, RtBound bound, RtProcSubdivFunc refineproc, RtProcFreeFunc freeproc) {}
RtVoid RiProcFree(RtPointer data) {}
RtVoid RiProcDelayedReadArchive(RtPointer data, RtFloat detail) {}
RtVoid RiProcRunProgram(RtPointer data, RtFloat detail) {}
RtVoid RiProcDynamicLoad(RtPointer data, RtFloat detail) {}
RtVoid RiGeometryV(RtToken type, RtInt count, RtToken tokens[], RtPointer values[]) {}
RtVoid RiSolidBegin(RtToken type) {}
RtVoid RiSolidEnd() {}
RtObjectHandle RiObjectBegin() {return 0;}
RtVoid RiObjectEnd() {}
RtVoid RiObjectInstance(RtObjectHandle handle) {}
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
