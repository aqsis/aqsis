// hairgen procedural
// Copyright (C) 2008 Christopher J. Foster [chris42f (at) gmail (d0t) com]
//
// This software is licensed under the GPLv2 - see the file COPYING for details.

#include <iostream>
#include <string>

#include <aqsis/smartptr.h>
#include <aqsis/ribparser.h>

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
class PrimVarInserter : public Aqsis::IqRibParamList
{
	private:
		PrimVars& m_primVars;
	public:
		PrimVarInserter(PrimVars& primVars)
			: m_primVars(primVars)
		{ }

		// inherited
		virtual void append(const Aqsis::CqPrimvarToken& token,
				const Aqsis::TqRiIntArray& value)
		{
			// string arrays aren't handled.
			g_errStream << "hairgen: integer primvars not yet handled - "
				<< token.name() << " discarded\n";
		}

		virtual void append(const Aqsis::CqPrimvarToken& token,
				const Aqsis::TqRiFloatArray& value)
		{
			m_primVars.append(token, value);
		}

		virtual void append(const Aqsis::CqPrimvarToken& token,
				const Aqsis::TqRiStringArray& value)
		{
			// string arrays aren't handled.
			g_errStream << "hairgen: string primvars not yet handled - "
				<< token.name() << " discarded\n";
		}
};


//------------------------------------------------------------------------------
/** Handler for RiPointsPolygons requests.
 *
 * Grabs the PointsPolygons data, and puts it into a EmitterMesh object.
 */
class PointsPolygonsRequest : public Aqsis::IqRibRequest
{
	private:
		boost::shared_ptr<EmitterMesh>& m_emitter;
		TqInt m_numHairs;
	public:
		PointsPolygonsRequest(boost::shared_ptr<EmitterMesh>& emitter, int numHairs)
			: m_emitter(emitter),
			m_numHairs(numHairs)
		{ }

		void handleRequest(Aqsis::CqRibParser& parser)
		{
			// number of verts per polygon
			const Aqsis::TqRiIntArray& numVerts = parser.getIntArray();
			// indices for vertice into the primvars of type vertex or varying
			const Aqsis::TqRiIntArray& vertIndices = parser.getIntArray();
			// Handle all primvars
			boost::shared_ptr<PrimVars> primVars(new PrimVars());
			PrimVarInserter pList(*primVars);
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
class CurvesRequest : public Aqsis::IqRibRequest
{
	private:
		boost::shared_ptr<ParentHairs>& m_hairs;
	public:
		CurvesRequest(boost::shared_ptr<ParentHairs>& hairs)
			: m_hairs(hairs)
		{ }

		void handleRequest(Aqsis::CqRibParser& parser)
		{
			// Curve type - "linear" or "cubic"
			std::string typeStr = parser.getString();
			bool linear = typeStr == "linear";
			// Number of verts per curve
			const Aqsis::TqRiIntArray& numVerts = parser.getIntArray();
			// periodic curves - "periodic" or "nonperiodic"
			std::string periodicStr = parser.getString();
			bool periodic = periodicStr == "periodic";
			// Handle all primvars
			boost::shared_ptr<PrimVars> primVars(new PrimVars());
			PrimVarInserter pList(*primVars);
			parser.getParamList(pList);

			// We can't deal with periodic curves or interpolate when there's
			// less that four parents.
			if(periodic || static_cast<int>(numVerts.size()) <
					ParentHairs::m_parentsPerChild)
				return;

			m_hairs.reset(new ParentHairs(linear, numVerts, primVars));
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
inline void parseStream(std::istream& ribStream, Aqsis::CqRequestMap& requests)
{
	Aqsis::CqRibLexer lex(ribStream);
	Aqsis::CqRibParser parser(
		boost::shared_ptr<Aqsis::CqRibLexer>(&lex, Aqsis::nullDeleter),
		boost::shared_ptr<Aqsis::CqRequestMap>(&requests, Aqsis::nullDeleter),
		true);
	for(std::vector<Aqsis::CqPrimvarToken>::const_iterator
		i = Aqsis::g_standardVars.begin(),
		end = Aqsis::g_standardVars.end();
		i != end; ++i)
	{
		parser.declareVariable(*i);
	}

	bool parsing = true;
	while(parsing)
	{
		try
		{
			parsing = parser.parseNextRequest();
		}
		catch(Aqsis::XqParseError& e)
		{
			g_errStream << e << "\n";
		}
	}
}
