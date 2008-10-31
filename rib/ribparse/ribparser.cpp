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
 *
 * TODO: Standardize the error reporting
 */

#include "ribparser.h"

namespace Aqsis {

//------------------------------------------------------------------------------
// CqRibParser implementation

CqRibParser::CqRibParser(const boost::shared_ptr<CqRibLexer>& lexer,
				const boost::shared_ptr<IqRibRequestHandler>& requestHandler)
	: m_lex(lexer),
	m_requestHandler(requestHandler),
	m_floatArrayPool(),
	m_intArrayPool(),
	m_stringArrayPool()
{ }

const boost::shared_ptr<CqRibLexer>& CqRibParser::lexer()
{
	return m_lex;
}

bool CqRibParser::parseNextRequest()
{
	// skip up until the next request.  This guards against the case that a
	// previous request resulted in an exception which left the parser in an
	// odd state.
	CqRibToken tok = m_lex->get();
	while(tok.type() != CqRibToken::REQUEST)
	{
		if(tok.type() == CqRibToken::ENDOFFILE)
			return false;
		tok = m_lex->get();
	}
	// Mark array pools as free to use.
	m_floatArrayPool.markUnused();
	m_intArrayPool.markUnused();
	m_stringArrayPool.markUnused();
	// Invoke the request handler.
	m_requestHandler->handleRequest(tok.stringVal(), *this);
	return true;
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
const CqRibParser::TqIntArray& CqRibParser::getIntArray()
{
	consumeArrayBegin(*m_lex, "integer");

	TqIntArray& buf = m_intArrayPool.getBuf();
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

const CqRibParser::TqFloatArray& CqRibParser::getFloatArray()
{
	consumeArrayBegin(*m_lex, "float");

	TqFloatArray& buf = m_floatArrayPool.getBuf();
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

const CqRibParser::TqStringArray& CqRibParser::getStringArray()
{
	consumeArrayBegin(*m_lex, "string");

	TqStringArray& buf = m_stringArrayPool.getBuf();
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

const CqRibParser::TqIntArray& CqRibParser::getIntParam()
{
	if(m_lex->peek().type() == CqRibToken::INTEGER)
	{
		TqIntArray& buf = m_intArrayPool.getBuf();
		buf.push_back(m_lex->get().intVal());
		return buf;
	}
	return getIntArray();
}

const CqRibParser::TqFloatArray& CqRibParser::getFloatParam()
{
	switch(m_lex->peek().type())
	{
		case CqRibToken::INTEGER:
			{
				TqFloatArray& buf = m_floatArrayPool.getBuf();
				buf.push_back(m_lex->get().intVal());
				return buf;
			}
		case CqRibToken::FLOAT:
			{
				TqFloatArray& buf = m_floatArrayPool.getBuf();
				buf.push_back(m_lex->get().floatVal());
				return buf;
			}
		default:
			return getFloatArray();
	}
}

const CqRibParser::TqStringArray& CqRibParser::getStringParam()
{
	if(m_lex->peek().type() == CqRibToken::STRING)
	{
		// special case where next token is a single string.
		TqStringArray& buf = m_stringArrayPool.getBuf();
		buf.push_back(m_lex->get().stringVal());
		return buf;
	}
	return getStringArray();
}

const void CqRibParser::getParamList(IqRibParamListHandler& paramHandler)
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
						"parameter name string expected in param list, got "
						<< m_lex->peek());
		}
		// Note: we NEED to copy paramName into a temporary variable here.
		// The current token is returned by reference from the lexer, and will
		// be overwritten due to the user calling any of the parser getter
		// functions inside readParameter().
		std::string paramName = m_lex->get().stringVal();
		paramHandler.readParameter(paramName, *this);
	}
}

} // namespace Aqsis
