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
 * \brief RIB parser implementation
 * \author Chris Foster [chris42f (at) gmail (d0t) com]
 */

#include "ribparser.h"

namespace Aqsis {

//------------------------------------------------------------------------------
// CqRibParser implementation

CqRibParser::CqRibParser(const boost::shared_ptr<CqRibLexer>& lexer,
		const boost::shared_ptr<CqRequestMap>& requests,
		bool ignoreUnrecognized)
	: m_lex(lexer),
	m_requests(requests),
	m_ignoreUnrecognized(ignoreUnrecognized),
	m_floatArrayPool(),
	m_intArrayPool(),
	m_stringArrayPool()
{ }

bool CqRibParser::parseNextRequest()
{
	while(true)
	{
		// skip up until the next request.
		CqRibToken tok = m_lex->get();
		while(tok.type() != CqRibToken::REQUEST)
		{
			if(tok.type() == CqRibToken::ENDOFFILE)
				return false;
			tok = m_lex->get();
		}
		// find a handler for the current request
		if(IqRibRequest* req = m_requests->find(tok.stringVal()))
		{
			// Mark array pools as free to use.
			m_floatArrayPool.markUnused();
			m_intArrayPool.markUnused();
			m_stringArrayPool.markUnused();
			// Invoke the request handler.
			req->handleRequest(*this);
			return true;
		}
		else
		{
			// If the request isn't found, throw if desired, otherwise
			// continue on to the next request in the stream.
			if(!m_ignoreUnrecognized)
			{
				AQSIS_THROW(XqParseError,
						"urecognized RIB request " << tok.stringVal()
						<< " (" << m_lex->pos() << ")");
			}
		}
	}
}

TqInt CqRibParser::getInt()
{
	CqRibToken tok = m_lex->get();
	if(tok.type() != CqRibToken::INTEGER)
		AQSIS_THROW(XqParseError, "Found " << tok << " expected INTEGER");
	return tok.intVal();
}

TqFloat CqRibParser::getFloat()
{
	CqRibToken tok = m_lex->get();
	if(tok.type() != CqRibToken::FLOAT)
		AQSIS_THROW(XqParseError, "Found " << tok << " expected FLOAT");
	return tok.floatVal();
}

std::string CqRibParser::getString()
{
	CqRibToken tok = m_lex->get();
	if(tok.type() != CqRibToken::STRING)
		AQSIS_THROW(XqParseError, "Found " << tok << " expected STRING");
	return tok.stringVal();
}

namespace {
/** Helper function to consume an ARRAY_BEGIN token and throw an appropriate
 * error if a different token is encountered.
 */
inline void consumeArrayBegin(CqRibLexer& lex, const char* arrayType)
{
	CqRibToken tok = lex.get();
	if(tok.type() != CqRibToken::ARRAY_BEGIN)
	{
		AQSIS_THROW(XqParseError,
				lex.pos() << ": expected " << arrayType
				<< " array, got token " << tok);
	}
}
}

/* Functions for reading arrays.
 *
 * The code duplication between the get*Array() functions here is somewhat
 * alarming, but it's tricky to do something about it without the code becoming
 * an opaque mess of templates.
 *
 * At the very least, we'd need to make the tok.intVal(), tok.floatVal(),
 * tok.stringVal() into a template or overloaded functions taking a reference
 * so that the correct version could be called in a templated readArray().
 * After that we'd still be left with the float case in which it's acceptable
 * for the token to be *either* an integer *or* a float.
 */
const TqRiIntArray& CqRibParser::getIntArray()
{
	consumeArrayBegin(*m_lex, "integer");

	TqRiIntArray& buf = m_intArrayPool.getBuf();
	bool parsing = true;
	while(parsing)
	{
		CqRibToken tok = m_lex->get();
		switch(tok.type())
		{
			case CqRibToken::INTEGER:
				buf.push_back(tok.intVal());
				break;
			case CqRibToken::ARRAY_END:
				parsing = false;
				break;
			default:
				AQSIS_THROW(XqParseError,
						m_lex->pos() << ": unexpected token " << tok
						<< "while reading integer array");
				break;
		}
	}
	return buf;
}

const TqRiFloatArray& CqRibParser::getFloatArray()
{
	consumeArrayBegin(*m_lex, "float");

	TqRiFloatArray& buf = m_floatArrayPool.getBuf();
	bool parsing = true;
	while(parsing)
	{
		CqRibToken tok = m_lex->get();
		switch(tok.type())
		{
			case CqRibToken::INTEGER:
				buf.push_back(tok.intVal());
				break;
			case CqRibToken::FLOAT:
				buf.push_back(tok.floatVal());
				break;
			case CqRibToken::ARRAY_END:
				parsing = false;
				break;
			default:
				AQSIS_THROW(XqParseError,
						m_lex->pos() << ": unexpected token " << tok
						<< "while reading float array");
				break;
		}
	}
	return buf;
}

const TqRiStringArray& CqRibParser::getStringArray()
{
	consumeArrayBegin(*m_lex, "string");

	TqRiStringArray& buf = m_stringArrayPool.getBuf();
	bool parsing = true;
	while(parsing)
	{
		CqRibToken tok = m_lex->get();
		switch(tok.type())
		{
			case CqRibToken::STRING:
				buf.push_back(tok.stringVal());
				break;
			case CqRibToken::ARRAY_END:
				parsing = false;
				break;
			default:
				AQSIS_THROW(XqParseError,
						m_lex->pos() << ": unexpected token " << tok
						<< "while reading string array");
				break;
		}
	}
	return buf;
}

const void CqRibParser::getParamList(IqRibParamList& paramList)
{
	while(true)
	{
		switch(m_lex->peek().type())
		{
			case CqRibToken::REQUEST:
			case CqRibToken::ENDOFFILE:
				// If we get to the next request or end of file, return since
				// we're done parsing the parameter list
				return;
			case CqRibToken::STRING:
				break;
			default:
				AQSIS_THROW(XqParseError,
						"name/type string expected in parameter list");
		}
		// get a string representing the name of the param list.
		CqPrimvarToken primvarTok(m_lex->get().stringVal().c_str());

		// Now parse the value associated with the parameter string.
		switch(primvarTok.type())
		{
			//--------------------------------------------------
			// Parameters represented as floats
			case type_float:
				if(m_lex->peek().type() == CqRibToken::FLOAT)
				{
					// deal with the special case where the next token is a
					// single float rather than an array.
					TqRiFloatArray& buf = m_floatArrayPool.getBuf();
					buf.push_back(m_lex->get().floatVal());
					paramList.append(primvarTok, buf);
					break;
				}
				// intentional case fallthrough.
			case type_point:
			case type_vector:
			case type_normal:
			case type_hpoint:
			case type_matrix:
			case type_color:
				// Any of the above types need to correspond to a float array.
				paramList.append(primvarTok, getFloatArray());
				break;
			//--------------------------------------------------
			// Parameters represented as integers
			case type_integer:
				if(m_lex->peek().type() == CqRibToken::INTEGER)
				{
					TqRiIntArray& buf = m_intArrayPool.getBuf();
					buf.push_back(m_lex->get().intVal());
					paramList.append(primvarTok, buf);
				}
				else
					paramList.append(primvarTok, getIntArray());
				break;
			//--------------------------------------------------
			// Parameters represented as strings
			case type_string:
				if(m_lex->peek().type() == CqRibToken::STRING)
				{
					// special case where next token is a single string.
					TqRiStringArray& buf = m_stringArrayPool.getBuf();
					buf.push_back(m_lex->get().stringVal());
					paramList.append(primvarTok, buf);
				}
				else
					paramList.append(primvarTok, getStringArray());
				break;
			//--------------------------------------------------
			// Invalid parameters.
			case type_bool:
			case type_invalid:
			case type_void:
			case type_triple:
			case type_sixteentuple:
			default:
				// Any of the types above aren't really valid RIB, so we
				// disallow them here.
				AQSIS_THROW(XqParseError, "invalid token type '"
						<< primvarTok.type() << "' in parameter list");
				break;
		}
	}
}

//------------------------------------------------------------------------------
void CqRequestMap::add(const std::string& name, IqRibRequest* request)
{
	m_requests[name].reset(request);
}

} // namespace Aqsis
