// hairgen procedural
// Copyright (C) 2008 Christopher J. Foster [chris42f (at) gmail (d0t) com]
//
// This software is licensed under the GPLv2 - see the file COPYING for details.

#include <iostream>
#include <map>
#include <string>

#include <aqsis/util/smartptr.h>
#include <aqsis/riutil/errorhandler.h>
#include <aqsis/riutil/ribparser.h>
#include <aqsis/riutil/ricxxutil.h>
#include <aqsis/riutil/tokendictionary.h>

#include "parenthairs.h"
#include "primvar.h"
#include "util.h"

/// Extracts Curves and Polygons data from a RIB stream.
///
/// The Curves data gets put into a ParentHairs object.
/// The PointsPolygons data gets put into an EmitterMesh object.
class HairgenApi : public Aqsis::StubRenderer
{
	private:
		boost::shared_ptr<EmitterMesh>& m_emitter;
		int m_numHairs;
		boost::shared_ptr<ParentHairs>& m_hairs;
		const HairModifiers& m_hairModifiers;
	public:
		HairgenApi(boost::shared_ptr<EmitterMesh>& emitter,
				   int numHairs, boost::shared_ptr<ParentHairs>& hairs,
				   const HairModifiers& hairModifiers)
			: m_emitter(emitter),
			m_numHairs(numHairs),
			m_hairs(hairs),
			m_hairModifiers(hairModifiers)
		{ }

		virtual RtVoid Curves(RtConstToken type, const IntArray& nvertices,
							  RtConstToken wrap, const ParamList& pList)
		{
			// We can't deal with periodic curves or interpolate when there's
			// less than a certian number of parent hairs.
			if((int)nvertices.size() < ParentHairs::m_parentsPerChild
			   || strcmp(wrap, "periodic")==0)
				return;
			bool linear = strcmp(type, "linear") == 0;
			boost::shared_ptr<PrimVars> params(new PrimVars(pList));
			m_hairs.reset(new ParentHairs(linear, nvertices,
										  params, m_hairModifiers));
		}

		virtual RtVoid PointsPolygons(const IntArray& nverts,
									  const IntArray& verts,
									  const ParamList& pList)
		{
			// Handle all primvars
			boost::shared_ptr<PrimVars> params(new PrimVars(pList));
			m_emitter.reset(new EmitterMesh(nverts, verts,
						params, m_numHairs));
		}
};

class HairgenApiServices : public Aqsis::StubRendererServices
{
	private:
		HairgenApi m_api;
		Aqsis::CqTokenDictionary m_tokenDict;
		boost::shared_ptr<Aqsis::RibParser> m_parser;

		class ErrorHandler : public Ri::ErrorHandler
		{
			public:
				ErrorHandler() : Ri::ErrorHandler(Warning) {}
			protected:
				virtual void sendError(int code, const std::string& message)
				{
					std::ostream& out = g_errStream;
					switch(errorCategory(code))
					{
						case Debug:   out << "DEBUG: ";    break;
						case Info:    out << "INFO: ";     break;
						case Warning: out << "WARNING: ";  break;
						case Error:   out << "ERROR: ";    break;
						case Severe:  out << "CRITICAL: "; break;
						case Message: out << "INFO: ";     break;
					}
					out << message << std::endl;
				}
		};
		ErrorHandler m_errHandler;

	public:
		HairgenApiServices(boost::shared_ptr<EmitterMesh>& emitter,
				   int numHairs, boost::shared_ptr<ParentHairs>& hairs,
				   const HairModifiers& hairModifiers)
			: m_api(emitter, numHairs, hairs, hairModifiers),
			m_tokenDict(true)
		{
			m_parser.reset(Aqsis::RibParser::create(*this));
		}

		virtual Ri::ErrorHandler& errorHandler()
		{
			return m_errHandler;
		}

        virtual Ri::TypeSpec getDeclaration(RtConstToken token,
                                        const char** nameBegin = 0,
                                        const char** nameEnd = 0) const
		{
            Ri::TypeSpec spec = Aqsis::parseDeclaration(token, nameBegin, nameEnd);
            if(spec.type == Ri::TypeSpec::Unknown)
            {
                // FIXME: Yuck, ick, ew!  Double parsing here :/
                spec = toTypeSpec(m_tokenDict.parseAndLookup(token));
            }
            return spec;
		}

		virtual Ri::Renderer& firstFilter()
		{
			return m_api;
		}

		virtual void parseRib(std::istream& ribStream, const char* name,
							  Ri::Renderer& context)
		{
			m_parser->parseStream(ribStream, name, context);
		}
		using Ri::RendererServices::parseRib;
};

