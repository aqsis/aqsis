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
	m_stringArrayPool(),
	m_currParamList()
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

namespace {
EqRiParamType paramForVarType(EqVariableType varType)
{
	switch(varType)
	{
		case type_float:
		case type_point:
		case type_color:
		case type_triple:
		case type_hpoint:
		case type_normal:
		case type_vector:
		case type_matrix:
		case type_sixteentuple:
			return ParamType_Float;
		case type_string:
			return ParamType_String;
		case type_bool:
		case type_integer:
			return ParamType_Int;
		case type_invalid:
		case type_void:
		default:
			assert(0 && "no associated type");
			return ParamType_Int;
	}
}
}

const TqRiParamList& CqRibParser::getParamList()
{
	m_currParamList.clear();
	while(true)
	{
		switch(m_lex->peek().type())
		{
			case CqRibToken::REQUEST:
			case CqRibToken::ENDOFFILE:
				return m_currParamList;
			case CqRibToken::STRING:
				break;
			default:
				AQSIS_THROW(XqParseError,
						"name/type string expected in parameter list");
		}
		// get a string representing the name of the param list.
		std::string name = getString();
		CqPrimvarToken tok(name.c_str());
		switch(paramForVarType(tok.type()))
		{
			case ParamType_Int:
				if(m_lex->peek().type() == CqRibToken::ARRAY_BEGIN)
					m_currParamList.push_back(
							CqRequestParam(tok, ParamType_IntArray, getIntArray()));
				else
					m_currParamList.push_back(
							CqRequestParam(tok, ParamType_Int, getInt()));
				break;
			case ParamType_Float:
				if(m_lex->peek().type() == CqRibToken::ARRAY_BEGIN)
					m_currParamList.push_back(
							CqRequestParam(tok, ParamType_FloatArray, getFloatArray()));
				else
					m_currParamList.push_back(
							CqRequestParam(tok, ParamType_Float, getFloat()));
				break;
			case ParamType_String:
				if(m_lex->peek().type() == CqRibToken::ARRAY_BEGIN)
					m_currParamList.push_back(
							CqRequestParam(tok, ParamType_StringArray, getStringArray()));
				else
					m_currParamList.push_back(
							CqRequestParam(tok, ParamType_String, getString()));
				break;
			default:
				assert(0 && "invalid param type");
		}
	}
}

//------------------------------------------------------------------------------
void CqRequestMap::add(IqRibRequest* request)
{
	m_requests[request->name()].reset(request);
}

} // namespace Aqsis
