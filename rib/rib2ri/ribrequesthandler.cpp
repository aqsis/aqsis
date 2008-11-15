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

#include "ri.h"
#include "ribparser.h"

namespace Aqsis
{

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
	m_requestNames.reserve(numRequests, 0);
	m_requestNameHashes.reserve(numRequests, 0);
	m_requestHandlers.reserve(numRequests, 0);
	for(TqInt i = 0; i < numRequests; ++i)
	{
		m_requestNameHashes.push_back(boost::hash_value(requestNames[i]));
		m_requestNames.push_back(requestNames[i]);
		m_requestNameHashes.push_back(boost::hash_value(requestHandlers[i]));
	}
}

virtual void CqRibRequestHandler::handleRequest(const std::string& requestName,
		CqRibParser& parser)
{
	std::vector<TqHash>::const_iterator pos = std::lower_bound(
			m_requestNameHashes.begin(), m_requestNameHashes.end(),
			boost::hash_value(requestName));
	int offset = pos - m_requestNameHashes.begin();
	for(numRequests = m_requestNames.size(); offset < numRequests
			&& ; ++offset)
	{
		if(m_requestNames[offset] != requestName)
		(*this).*(m_requestHandlers[offset])(parser);
		return;
	}
	AQSIS_THROW(XqParseError, "unrecognized request");
}

//--------------------------------------------------
// Conversion shims from CqRibParser types into RI types.

namespace {

inline RtToken toRiType(const std::string& str)
{
	return const_cast<RtToken>(str.c_str());
}

inline RtFloat* toRiType(const CqRibParser::TqFloatArray& a)
{
	return const_cast<RtFloat*>(&a[0]);
}

inline RtInt* toRiType(const CqRibParser::TqIntArray& a)
{
	return const_cast<RtInt*>(&a[0]);
}

inline RtInt toRiType(TqInt i)
{
	return i;
}

inline RtFloat toRiType(TqFloat f)
{
	return f;
}

/** Dummy type standing in for a filter function name.
 *
 * Using this type causes the correct toRiType() conversion shim to be
 * selected such that a RtFilterFunc is sent to the C interface.
 */
class CqFilterFuncString : public std::string {};

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
		AQSIS_THROW(XqParseError, "unknown filter function \"" << filterName << "\"");
		return 0;
	}
}

class CqProceduralFuncString : public std::string {};

inline RtFunc toRiType(const CqProceduralFuncString& procName)
{
	if(procName == "DelayedReadArchive") return &::RiProcDelayedReadArchive;
    else if(procName == "RunProgram")    return &::RiProcRunProgram;
    else if(procName == "DynamicLoad")   return &::RiProcDynamicLoad;
    else if(procName == "RiProcFree")    return &::RiProcFree;
	else
	{
		AQSIS_THROW(XqParseError, "unknown procedural function \""
				<< filterName << "\"");
		return 0;
	}
}

class CqErrorHandlerString : public std::string {};

inline RtFunc toRiType(const CqErrorHandlerString& handlerName)
{
	if(handlerName == "ignore")      return &::RiErrorIgnore;
    else if(handlerName == "print")  return &::RiErrorPrint;
    else if(handlerName == "abort")  return &::RiErrorAbort;
	else
	{
		AQSIS_THROW(XqParseError, "unknown error handler function \""
				<< handlerName << "\"");
		return 0;
	}
}

/// Holder for CqRibParser::TqFloatArray used for eventual RtMatrix conversion.
struct SqRtMatrixHolder
{
	const CqRibParser::TqFloatArray& matrix;
	CqRtMatrixHolder(const CqRibParser::TqFloatArray& matrix) : matrix(matrix) { }
};

inline RtMatrix toRiType(const SqRtMatrixHolder& matrixHolder)
{
	if(matrixHolder.matrix.size() != 16)
		AQSIS_THROW(XqParseError, "RtMatrix must have 16 elements");
	return reinterpret_cast<RtMatrix>(const_cast<TqFloat*>(&matrixHolder[0]));
}

inline RtBasis& toRiType(const RtBasis* basisPtr)
{
	return *const_cast<RtBasis*>(basisPtr);
}



} // unnamed namespace


//--------------------------------------------------
class CqStringToBasis : public IqStringToBasis
{
	public:
		virtual CqRibParser::TqBasis* getBasis(const std::string& name) const
		{
			if(name == "bezier")           return &::RiBezierBasis;
			else if(name == "b-spline")    return &::RiBSplineBasis;
			else if(name == "catmull-rom") return &::RiCatmullRomBasis;
			else if(name == "hermite")     return &::RiHermiteBasis;
			else if(name == "power")       return &::RiPowerBasis;
			else
			{
				AQSIS_THROW(XqParseError, "unknown basis name \""
						<< name << "\"");
			}
		}
};

//--------------------------------------------------
// Handlers for extracting requests from a RIB stream.
void CqRibRequestHandler::handleBasis(CqRibParser& parser)
{
	// Handle multiple forms of the Basis request:
	//
	// Basis (string | float_array) int (string | float_array) int
	//
	// The strings are converted into RtBasis.

	RtBasis uBasis = getBasis(parser);
	TqInt uBasisStep = parser.getInt();
	RtBasis vBasis = getBasis(parser);
	TqInt vBasisStep = parser.getInt();

	RiBasis(uBasis, uBasisStep, vBasis, vBasisStep);
}

void CqRibRequestHandler::handleVersion(CqRibParser& parser)
{
	TqFloat version = parser.getFloat();
	// Don't do anything with the version number; just blunder on regardless.
	// Probably only worth supporting if Pixar started publishing new versions
	// of the standard again...
}

void CqRibRequestHandler::handleColor(CqRibParser& parser)
{
	const CqRibParser::TqFloatArray& col = parser.getColor(m_numColorComps);
	RiColor(&col[0]);
}

void CqRibRequestHandler::handleOpacity(CqRibParser& parser)
{
	const CqRibParser::TqFloatArray& col = parser.getColor(m_numColorComps);
	RiOpacity(&col[0]);
}

void CqRibRequestHandler::handleCurves(CqRibParser& parser)
{
	std::string type = parser.getString();
	const Aqsis::TqRiIntArray& numVerts = parser.getIntArray();
	std::string periodic = parser.getString();

	CqParamList paramList;
	parser.getParamList(paramList);

	RiCurvesV(type.c_str(), numVerts.size(), &numVerts[0], periodic.c_str(),
			paramList.count(), paramList.tokens(), paramList.values());
}

// #include "requesthandler_definitions.cpp"

} // namespace Aqsis
