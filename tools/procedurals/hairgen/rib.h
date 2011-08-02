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
		Aqsis::TokenDict m_tokenDict;
		boost::shared_ptr<Aqsis::RibParser> m_parser;

		class ErrorHandler : public Ri::ErrorHandler
		{
			public:
				ErrorHandler() : Ri::ErrorHandler(Warning) {}
			protected:
				virtual void dispatch(int code, const std::string& message)
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
			m_tokenDict()
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
			return m_tokenDict.lookup(token, nameBegin, nameEnd);
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

