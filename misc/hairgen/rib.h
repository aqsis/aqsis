// hairgen procedural
// Copyright (C) 2008 Christopher J. Foster [chris42f (at) gmail (d0t) com]
//
// This software is licensed under the GPLv2 - see the file COPYING for details.

#include <iostream>
#include <map>
#include <string>

#include <smartptr.h>
#include <ribparser.h>
#include <tokendictionary.h>

#include "parenthairs.h"
#include "primvar.h"
#include "util.h"

//------------------------------------------------------------------------------
/** RIB parameter collection from libribparse --> PrimVars container.
 *
 * This class collects primvars as they are parsed by Aqsis::CqRibParser, and
 * stuffs them into a PrimVars container.  As such, it currently supports only
 * float primvars.
 */
class PrimVarInserter : public Aqsis::IqRibParamListHandler
{
	private:
		PrimVars& m_primVars;
		const Aqsis::CqTokenDictionary& m_tokenDict;
	public:
		PrimVarInserter(PrimVars& primVars, const Aqsis::CqTokenDictionary& tokenDict)
			: m_primVars(primVars),
			m_tokenDict(tokenDict)
		{ }

		// inherited
		virtual void readParameter(const std::string& name, Aqsis::CqRibParser& parser)
		{
			Aqsis::CqPrimvarToken token;
			try
			{
				token = m_tokenDict.parseAndLookup(name);
			}
			catch(Aqsis::XqValidation& e)
			{
				g_errStream << "hairgen: " << e.what() << "\n";
			}
			if(token.storageType() == Aqsis::type_float)
				m_primVars.append(token, parser.getFloatParam());
			else
				g_errStream << "hairgen: primvar not handled: " << token.name() << " discarded\n";
		}
};


//------------------------------------------------------------------------------
/** Handler for RiPointsPolygons requests.
 *
 * Grabs the PointsPolygons data, and puts it into a EmitterMesh object.
 */
class PointsPolygonsRequestHandler : public Aqsis::IqRibRequestHandler
{
	private:
		boost::shared_ptr<EmitterMesh>& m_emitter;
		TqInt m_numHairs;
		Aqsis::CqTokenDictionary m_tokenDict;
	public:
		PointsPolygonsRequestHandler(boost::shared_ptr<EmitterMesh>& emitter, int numHairs)
			: m_emitter(emitter),
			m_numHairs(numHairs),
			m_tokenDict(true)
		{ }

		void handleRequest(const std::string& name, Aqsis::CqRibParser& parser)
		{
			if(name != "PointsPolygons")
				return;
			// number of verts per polygon
			const IntArray& numVerts = parser.getIntArray();
			// indices for vertice into the primvars of type vertex or varying
			const IntArray& vertIndices = parser.getIntArray();
			// Handle all primvars
			boost::shared_ptr<PrimVars> primVars(new PrimVars());
			PrimVarInserter pList(*primVars, m_tokenDict);
			parser.getParamList(pList);

			m_emitter.reset(new EmitterMesh(numVerts, vertIndices,
						primVars, m_numHairs));
		}
};

//------------------------------------------------------------------------------
/** Handler for RiCurves requests.
 *
 * Grabs the Curves data, and puts it into a ParentHairs object.
 */
class CurvesRequestHandler : public Aqsis::IqRibRequestHandler
{
	private:
		boost::shared_ptr<ParentHairs>& m_hairs;
		const HairModifiers& m_hairModifiers;
		Aqsis::CqTokenDictionary m_tokenDict;
	public:
		CurvesRequestHandler(boost::shared_ptr<ParentHairs>& hairs,
				const HairModifiers& hairModifiers)
			: m_hairs(hairs),
			m_hairModifiers(hairModifiers),
			m_tokenDict(true)
		{ }

		void handleRequest(const std::string& name, Aqsis::CqRibParser& parser)
		{
			if(name != "Curves")
				return;
			// Curve type - "linear" or "cubic"
			std::string typeStr = parser.getString();
			bool linear = typeStr == "linear";
			// Number of verts per curve
			const IntArray& numVerts = parser.getIntArray();
			// periodic curves - "periodic" or "nonperiodic"
			std::string periodicStr = parser.getString();
			bool periodic = periodicStr == "periodic";
			// Handle all primvars
			boost::shared_ptr<PrimVars> primVars(new PrimVars());
			PrimVarInserter pList(*primVars, m_tokenDict);
			parser.getParamList(pList);

			// We can't deal with periodic curves or interpolate when there's
			// less that four parents.
			if(periodic || static_cast<int>(numVerts.size()) <
					ParentHairs::m_parentsPerChild)
				return;

			m_hairs.reset(new ParentHairs(linear, numVerts, primVars, m_hairModifiers));
		}
};


//------------------------------------------------------------------------------
/** Parse a RIB stream using the provided set of request handlers.
 *
 * \param ribStream - RIB input stream.
 * \param requests - A set of RI request handlers to be invoked by the RIB
 *                   parser.  Only handlers for the particular requests of
 *                   interest should be included.
 */
inline void parseStream(std::istream& ribStream,
		Aqsis::IqRibRequestHandler& requestHandler)
{
	Aqsis::CqRibLexer lex(ribStream);
	Aqsis::CqRibParser parser(
		boost::shared_ptr<Aqsis::CqRibLexer>(&lex, Aqsis::nullDeleter),
		boost::shared_ptr<Aqsis::IqRibRequestHandler>(&requestHandler, Aqsis::nullDeleter) );

	bool parsing = true;
	while(parsing)
	{
		try
		{
			parsing = parser.parseNextRequest();
		}
		catch(Aqsis::XqParseError& e)
		{
			g_errStream << "hairgen: " << e << "\n";
		}
	}
}
