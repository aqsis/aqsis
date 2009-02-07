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

/** \file
 *
 * \brief RIB request handler implementation for aqsis.
 * \author Chris Foster [chris42f (at) g mail (d0t) com]
 */

#include "ribrequesthandler.h"

#include <cstring>  // for strcpy

#include "ri.h"
#include "ribparser.h"

namespace Aqsis
{

namespace {

//------------------------------------------------------------------------------
class CqParamListHandler : public IqRibParamListHandler
{
	private:
		const CqTokenDictionary& m_tokenDict;
		std::vector<std::string> m_tokenStorage;
		std::vector<RtToken> m_tokens;
		std::vector<RtPointer> m_values;
		std::vector<std::vector<RtToken> > m_stringValues;
		TqInt m_countP;

	public:
		CqParamListHandler(const CqTokenDictionary& tokenDict) :
			m_tokenDict(tokenDict),
			m_tokenStorage(),
			m_tokens(),
			m_values(),
			m_stringValues(),
			m_countP(-1)
		{ }

		virtual void readParameter(const std::string& name, IqRibParser& parser)
		{
			CqPrimvarToken tok;
			try
			{
				tok = m_tokenDict.parseAndLookup(name);
			}
			catch(XqParseError& e)
			{
				throw;
			}
			catch(XqValidation& e)
			{
				AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax, e.what());
			}
			switch(tok.storageType())
			{
				case type_integer:
					m_values.push_back( reinterpret_cast<RtPointer>(
							const_cast<RtInt*>(&parser.getIntParam()[0])) );
					break;
				case type_float:
					{
						const IqRibParser::TqFloatArray& floats = parser.getFloatParam();
						m_values.push_back( reinterpret_cast<RtPointer>(
								const_cast<RtFloat*>(&floats[0])) );
						if(tok.name() == "P")
							m_countP = floats.size();
					}
					break;
				case type_string:
					{
						const IqRibParser::TqStringArray& strings = parser.getStringParam();
						m_stringValues.push_back(std::vector<RtToken>(strings.size(), 0));
						std::vector<RtToken>& stringsDest = m_stringValues.back();
						for(TqInt i = 0, end = strings.size(); i < end; ++i)
							stringsDest[i] = const_cast<RtToken>(strings[i].c_str());
						m_values.push_back( reinterpret_cast<RtPointer>(&stringsDest[0]) );
					}
					break;
				case type_invalid:
				default:
					assert(0 && "Unknown storage type; we should never get here.");
					return;
			}

			m_tokenStorage.push_back(name);
		}

		RtToken* tokens()
		{
			// Copy pointers to stored tokens into an array of RtToken.
			TqInt numTokens = m_tokenStorage.size();
			if(static_cast<TqInt>(m_tokens.size()) != numTokens)
			{
				m_tokens.resize(numTokens, 0);
				for(TqInt i = 0; i < numTokens; ++i)
					m_tokens[i] = const_cast<RtToken>(m_tokenStorage[i].c_str());
			}
			return numTokens > 0 ? &m_tokens[0] : 0;
		}

		RtPointer* values()
		{
			return m_values.size() > 0 ? &m_values[0] : 0;
		}

		RtInt count()
		{
			return m_tokens.size();
		}

		TqInt countP()
		{
			if(m_countP < 0)
				AQSIS_THROW_XQERROR(XqParseError, EqE_MissingData,
						"variable \"P\" not found in parameter list");
			return m_countP;
		}
};

} // unnamed namespace


//------------------------------------------------------------------------------
// CqRibRequestHandler implementation

CqRibRequestHandler::CqRibRequestHandler()
{
	// fill in the handler vectors somehow.
	// 
	// The following include should defines
	//   const char* requestNames[]
	//   TqRequestHandler requestHandlers[]
#	include "requestlists.inl"
	TqInt numRequests = sizeof(requestNames)/sizeof(const char*);
	for(TqInt i = 0; i < numRequests; ++i)
		m_requestHandlerMap[requestNames[i]] = requestHandlers[i];
}

void CqRibRequestHandler::handleRequest(const std::string& requestName,
		IqRibParser& parser)
{
	TqHandlerMap::const_iterator pos = m_requestHandlerMap.find(requestName);
	if(pos == m_requestHandlerMap.end())
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_BadToken,
				"unrecognized request \"" << requestName << "\"");
	}
	TqRequestHandler handler = pos->second;
	(this->*handler)(parser);
}

//--------------------------------------------------
// Conversion shims from various types into the RI types.
//
// We also define some types here to capture the RIB form of several
// specialized RI types and allow them to be converted into the correct RI
// type, since there's not a 1:1 mapping between types coming out of the RIB
// parser and types in the RI interface.

namespace {

inline RtInt toRiType(TqInt i)
{
	return i;
}

inline RtFloat toRiType(TqFloat f)
{
	return f;
}

inline RtToken toRiType(const std::string& str)
{
	return const_cast<RtToken>(str.c_str());
}

inline RtInt* toRiType(const IqRibParser::TqIntArray& a)
{
	return const_cast<RtInt*>(&a[0]);
}

inline RtFloat* toRiType(const IqRibParser::TqFloatArray& a)
{
	return const_cast<RtFloat*>(&a[0]);
}

inline RtArchiveCallback toRiType(RtArchiveCallback c)
{
	return c;
}

// Dummy types to capture a RIB type and allow it to be converted into the
// correct RI type by toRiType()
struct CqFilterFuncString : public std::string
{
	CqFilterFuncString(const std::string& str) : std::string(str) {}
};
struct CqErrorHandlerString : public std::string
{
	CqErrorHandlerString(const std::string& str) : std::string(str) {}
};

struct SqRtMatrixHolder
{
	const IqRibParser::TqFloatArray& matrix;
	SqRtMatrixHolder(const IqRibParser::TqFloatArray& matrix) : matrix(matrix) { }
};

/** A conversion class, converting IqRibParser::TqStringArray to
 * RtStringArray/RtTokenArray
 *
 * This is necessary, since RtTokenArray is an array of naked pointers; we need
 * to extract these from the TqStringArray and store them somewhere.
 */
struct SqRtTokenArrayHolder
{
	std::vector<RtToken> tokenStorage;

	/** Extract a vector of RtToken pointers from a vector of std::strings,
	 * storing them in the tokenStorage array.
	 */
	void convertToTokens(const IqRibParser::TqStringArray& strings)
	{
		tokenStorage.reserve(strings.size());
		for(IqRibParser::TqStringArray::const_iterator i = strings.begin(),
				end = strings.end(); i != end; ++i)
		{
			tokenStorage.push_back(const_cast<RtToken>(i->c_str()));
		}
	}

	SqRtTokenArrayHolder() { }

	SqRtTokenArrayHolder(const IqRibParser::TqStringArray& strings)
	{
		convertToTokens(strings);
	}

	// Get the number of tokens stored.
	TqInt size()
	{
		return tokenStorage.size();
	}
};

// Conversion shims for dummy types
inline RtFilterFunc toRiType(const CqFilterFuncString& filterName)
{
	if     (filterName == "box")         return &::RiBoxFilter;
	else if(filterName == "gaussian")    return &::RiGaussianFilter;
	else if(filterName == "triangle")    return &::RiTriangleFilter;
	else if(filterName == "mitchell")    return &::RiMitchellFilter;
	else if(filterName == "catmull-rom") return &::RiCatmullRomFilter;
	else if(filterName == "sinc")        return &::RiSincFilter;
	else if(filterName == "bessel")      return &::RiBesselFilter;
	else if(filterName == "disk")        return &::RiDiskFilter;
	else
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_BadToken,
			"unknown filter function \"" << filterName << "\"");
		return 0;
	}
}

inline RtErrorFunc toRiType(const CqErrorHandlerString& handlerName)
{
	if(handlerName == "ignore")      return &::RiErrorIgnore;
    else if(handlerName == "print")  return &::RiErrorPrint;
    else if(handlerName == "abort")  return &::RiErrorAbort;
	else
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_BadToken,
			"unknown error handler function \"" << handlerName << "\"");
		return 0;
	}
}

inline RtMatrix& toRiType(const SqRtMatrixHolder& matrixHolder)
{
	if(matrixHolder.matrix.size() != 16)
		AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
			"RtMatrix must have 16 elements");
	return *reinterpret_cast<RtMatrix*>(
			const_cast<TqFloat*>(&matrixHolder.matrix[0]));
}

inline RtToken* toRiType(SqRtTokenArrayHolder& stringArrayHolder)
{
	return &stringArrayHolder.tokenStorage[0];
}

} // unnamed namespace

//--------------------------------------------------

void CqRibRequestHandler::handleVersion(IqRibParser& parser)
{
	parser.getFloat();
	// Don't do anything with the version number; just blunder on regardless.
	// Probably only worth supporting if Pixar started publishing new versions
	// of the standard again...
}

//--------------------------------------------------

// Request handlers with autogenerated implementations.
#include "requesthandler_method_impl.inl"


//--------------------------------------------------

void CqRibRequestHandler::handleDeclare(IqRibParser& parser)
{
	// Collect arguments from parser.
	std::string name = parser.getString();
	std::string declaration = parser.getString();

	m_tokenDict.insert(CqPrimvarToken(declaration.c_str(), name.c_str()));

	// Call through to the C binding.
	RiDeclare(toRiType(name), toRiType(declaration));
}

void CqRibRequestHandler::handleDepthOfField(IqRibParser& parser)
{
	if(parser.peekNextType() == IqRibParser::Tok_RequestEnd)
	{
		// If called without arguments, reset to the default pinhole camera.
		RiDepthOfField(FLT_MAX, FLT_MAX, FLT_MAX);
	}
	else
	{
		// Collect arguments from parser.
		TqFloat fstop = parser.getFloat();
		TqFloat focallength = parser.getFloat();
		TqFloat focaldistance = parser.getFloat();

		// Call through to the C binding.
		RiDepthOfField(toRiType(fstop), toRiType(focallength), toRiType(focaldistance));
	}
}

void CqRibRequestHandler::handleColorSamples(IqRibParser& parser)
{
	// Collect arguments from parser.
	const IqRibParser::TqFloatArray& nRGB = parser.getFloatArray();
	const IqRibParser::TqFloatArray& RGBn = parser.getFloatArray();

	TqInt N = nRGB.size()/3;

	m_numColorComps = N;

	// Call through to the C binding.
	RiColorSamples(toRiType(N), toRiType(nRGB), toRiType(RGBn));
}

/** Handle either RiLightSourceV or RiAreaLightSourceV
 *
 * Both of these share the same RIB arguments and handler requirements, so the
 * code is consolidated in this function.
 *
 * \param riLightSourceFunc - Callback function for one of Ri{Area}LightSourceV
 * \param parser - parser from which to read the arguments
 */
void CqRibRequestHandler::handleLightSourceGeneral(
		TqLightSourceVFunc riLightSourceFunc, IqRibParser& parser)
{
	// Collect arguments from parser.
	std::string name = parser.getString();

	TqInt sequencenumber = 0;
	// Although not specified by the RISpec, the previous aqsis RIB parser
	// allowed lights to be specified by name as well as by a 'sequence
	// number'.  Here we retain that functionality (especially since there's
	// some test scenes which use it).
	std::string lightName;
	bool useLightName = false;
	if(parser.peekNextType() == IqRibParser::Tok_String)
	{
		lightName = parser.getString();
		useLightName = true;
	}
	else
		sequencenumber = parser.getInt();

	// Extract the parameter list
	CqParamListHandler paramList(m_tokenDict);
	parser.getParamList(paramList);

	// Call through to the C binding
	RtLightHandle lightHandle = (*riLightSourceFunc)(toRiType(name),
			paramList.count(), paramList.tokens(), paramList.values());

	// associate handle with the sequence number/name.
	if(lightHandle)
	{
		if(useLightName)
			m_namedLightMap[lightName] = lightHandle;
		else
			m_lightMap[sequencenumber] = lightHandle;
	}
}

void CqRibRequestHandler::handleLightSource(IqRibParser& parser)
{
	handleLightSourceGeneral(&RiLightSourceV, parser);
}

void CqRibRequestHandler::handleAreaLightSource(IqRibParser& parser)
{
	handleLightSourceGeneral(&RiAreaLightSourceV, parser);
}

void CqRibRequestHandler::handleIlluminate(IqRibParser& parser)
{
	// Collect arguments from parser.
	RtLightHandle lightHandle = 0;
	if(parser.peekNextType() == IqRibParser::Tok_String)
	{
		std::string name = parser.getString();
		TqNamedLightMap::const_iterator pos = m_namedLightMap.find(name);
		if(pos == m_namedLightMap.end())
			AQSIS_THROW_XQERROR(XqParseError, EqE_BadHandle,
					"undeclared light name \"" << name << "\"");
		lightHandle = pos->second;
	}
	else
	{
		TqInt sequencenumber = parser.getInt();
		TqLightMap::const_iterator pos = m_lightMap.find(sequencenumber);
		if(pos == m_lightMap.end())
			AQSIS_THROW_XQERROR(XqParseError, EqE_BadHandle,
					"undeclared light number " << sequencenumber);
		lightHandle = pos->second;
	}
	TqInt onoff = parser.getInt();

	// Call through to the C binding.
	RiIlluminate(lightHandle, onoff);
}

/** \brief Retrieve a spline basis array from the parser
 *
 * A spline basis array can be specified in two ways in a RIB stream: as an
 * array of 16 floats, or as a string indicating one of the standard bases.
 * This function returns the appropriate basis array, translating the string
 * representation into one of the standard arrays if necessary.
 *
 * \param parser - read input from here.
 */
RtBasis* CqRibRequestHandler::getBasis(IqRibParser& parser)
{
	switch(parser.peekNextType())
	{
		case IqRibParser::Tok_Array:
			{
				const CqRibParser::TqFloatArray& basis = parser.getFloatArray();
				if(basis.size() != 16)
					AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
						"basis array must be of length 16");
				// Note: This cast is a little ugly, but should only cause
				// problems in the *very* odd case that the alignment of
				// RtBasis and RtFloat* is different.
				return reinterpret_cast<RtBasis*>(const_cast<TqFloat*>(&basis[0]));
			}
		case IqRibParser::Tok_String:
			{
				std::string name = parser.getString();
				if(name == "bezier")           return &::RiBezierBasis;
				else if(name == "b-spline")    return &::RiBSplineBasis;
				else if(name == "catmull-rom") return &::RiCatmullRomBasis;
				else if(name == "hermite")     return &::RiHermiteBasis;
				else if(name == "power")       return &::RiPowerBasis;
				else
				{
					AQSIS_THROW_XQERROR(XqParseError, EqE_BadToken,
						"unknown basis \"" << name << "\"");
				}
			}
		default:
			AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
				"expected string or float array for basis");
			return 0;
	}
}

void CqRibRequestHandler::handleBasis(IqRibParser& parser)
{
	// Collect arguments from parser.
	RtBasis* ubasis = getBasis(parser);
	TqInt ustep = parser.getInt();
	RtBasis* vbasis = getBasis(parser);
	TqInt vstep = parser.getInt();

	// Call through to the C binding.
	RiBasis(*ubasis, ustep, *vbasis, vstep);
}

void CqRibRequestHandler::handleSubdivisionMesh(IqRibParser& parser)
{
	// Collect arguments from parser.
	std::string scheme = parser.getString();
	const IqRibParser::TqIntArray& nvertices = parser.getIntArray();
	const IqRibParser::TqIntArray& vertices = parser.getIntArray();

	TqInt nfaces = nvertices.size();

	// The following four arguments are optional.
	SqRtTokenArrayHolder tags;
	const IqRibParser::TqIntArray* nargs = 0;
	const IqRibParser::TqIntArray* intargs = 0;
	const IqRibParser::TqFloatArray* floatargs = 0;
	TqInt ntags = 0;

	if(parser.peekNextType() == IqRibParser::Tok_Array)
	{
		tags.convertToTokens(parser.getStringArray());
		nargs = &parser.getIntArray();
		intargs = &parser.getIntArray();
		floatargs = &parser.getFloatArray();

		// Check that the number of tags matches the number of arguments
		TqInt ntags = tags.size();
		if(static_cast<TqInt>(nargs->size()) != ntags*2)
		{
			AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
					"Invalid nargs length " << nargs->size() << "; expected length "
					"2*ntags = " << 2*ntags);
		}

		// Check that the argument arrays have length consistent with nargs
		TqInt intArgsLen = 0;
		TqInt floatArgsLen = 0;
		for(TqInt i = 0; i < ntags; ++i)
		{
			intArgsLen += (*nargs)[2*i];
			floatArgsLen += (*nargs)[2*i+1];
		}
		if(intArgsLen != static_cast<TqInt>(intargs->size()))
		{
			AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
					"Invalid intargs length " << intargs->size()
					<< "; expected length = " << intArgsLen);
		}
		if(floatArgsLen != static_cast<TqInt>(floatargs->size()))
		{
			AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
					"Invalid floatargs length " << floatargs->size()
					<< "; expected length = " << floatArgsLen);
		}
	}

	// Extract the parameter list
	CqParamListHandler paramList(m_tokenDict);
	parser.getParamList(paramList);

	// Call through to the C binding.
	RiSubdivisionMeshV(
			// Required args
			toRiType(scheme),
			toRiType(nfaces), toRiType(nvertices), toRiType(vertices),
			// Optional args
			ntags,
			ntags > 0 ? toRiType(tags) : NULL,
			nargs ? toRiType(*nargs) : NULL,
			intargs ? toRiType(*intargs) : NULL,
			floatargs ? toRiType(*floatargs) : NULL,
			// Parameter list
			paramList.count(), paramList.tokens(), paramList.values());
}

void CqRibRequestHandler::handleHyperboloid(IqRibParser& parser)
{
	// Collect all args as an array
	const IqRibParser::TqFloatArray& allArgs = parser.getFloatArray(7);

	// Collect arguments from parser.
	RtPoint* point1 = reinterpret_cast<RtPoint*>(const_cast<RtFloat*>(&allArgs[0]));
	RtPoint* point2 = reinterpret_cast<RtPoint*>(const_cast<RtFloat*>(&allArgs[3]));
	RtFloat thetamax = allArgs[6];

	// Extract the parameter list
	CqParamListHandler paramList(m_tokenDict);
	parser.getParamList(paramList);

	// Call through to the C binding.
	RiHyperboloidV(*point1, *point2, thetamax,
			paramList.count(), paramList.tokens(), paramList.values());
}

void CqRibRequestHandler::handleProcedural(IqRibParser& parser)
{
	// Collect arguments from parser.

	// get procedural subdivision function
	std::string procName = parser.getString();
	RtProcSubdivFunc subdivideFunc = 0;
	if(procName == "DelayedReadArchive") subdivideFunc = &::RiProcDelayedReadArchive;
    else if(procName == "RunProgram")    subdivideFunc = &::RiProcRunProgram;
    else if(procName == "DynamicLoad")   subdivideFunc = &::RiProcDynamicLoad;
	else
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_BadToken,
					"unknown procedural function \"" << procName << "\"");
	}

	// get argument string array.
	const IqRibParser::TqStringArray& args = parser.getStringArray();

	// Convert the string array to something passable as data arguments to the
	// builtin procedurals.
	//
	// We jump through a few hoops to meet the spec here.  The data argument to
	// the builtin procedurals should be interpretable as an array of RtString,
	// which we somehow also want to be free()'able.  If we choose to use
	// RiProcFree(), we must allocate it in one big lump.  Ugh.
	size_t dataSize = 0;
	TqInt numArgs = args.size();
	for(TqInt i = 0; i < numArgs; ++i)
	{
		dataSize += sizeof(RtString);   // one pointer for this entry
		dataSize += args[i].size() + 1; // and space for the string
	}
	RtPointer pdata = reinterpret_cast<RtPointer>(malloc(dataSize));
	RtString stringstart = reinterpret_cast<RtString>(
			reinterpret_cast<RtString*>(pdata) + numArgs);
	for(TqInt i = 0; i < numArgs; ++i)
	{
		reinterpret_cast<RtString*>(pdata)[i] = stringstart;
		std::strcpy(stringstart, args[i].c_str());
		stringstart += args[i].size() + 1;
	}

	// get procedural bounds
	const IqRibParser::TqFloatArray& bound = parser.getFloatArray();
	if(bound.size() != 6)
		AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
					"expected 6 elements in RtBound array");

	RiProcedural(pdata, toRiType(bound), subdivideFunc, &::RiProcFree);
}

void CqRibRequestHandler::handleObjectBegin(IqRibParser& parser)
{
	// The RIB identifier objects is an integer according to the RISpec, but
	// the previous parser also allowed strings.  See also notes in
	// generalHandleLightSource().
	if(parser.peekNextType() == IqRibParser::Tok_String)
	{
		std::string lightName = parser.getString();
		if(RtObjectHandle handle = RiObjectBegin())
			m_namedObjectMap[lightName] = handle;
	}
	else
	{
		TqInt sequenceNumber = parser.getInt();
		if(RtObjectHandle handle = RiObjectBegin())
			m_objectMap[sequenceNumber] = handle;
	}
}

void CqRibRequestHandler::handleObjectInstance(IqRibParser& parser)
{
	if(parser.peekNextType() == IqRibParser::Tok_String)
	{
		std::string name = parser.getString();
		TqNamedObjectMap::const_iterator pos = m_namedObjectMap.find(name);
		if(pos == m_namedObjectMap.end())
			AQSIS_THROW_XQERROR(XqParseError, EqE_BadHandle,
					"undeclared object name \"" << name << "\"");
		RiObjectInstance(pos->second);
	}
	else
	{
		TqInt sequencenumber = parser.getInt();
		TqObjectMap::const_iterator pos = m_objectMap.find(sequencenumber);
		if(pos == m_objectMap.end())
			AQSIS_THROW_XQERROR(XqParseError, EqE_BadHandle,
					"undeclared object number " << sequencenumber);
		RiObjectInstance(pos->second);
	}
}

} // namespace Aqsis
