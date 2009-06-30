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
 */

#include "ribparser_impl.h"

namespace Aqsis {

//------------------------------------------------------------------------------
// IqRibParser implementation.
boost::shared_ptr<IqRibParser> IqRibParser::create(
		const boost::shared_ptr<IqRibRequestHandler>& handler)
{
	return boost::shared_ptr<IqRibParser>(new CqRibParser(handler));
}


//------------------------------------------------------------------------------
// CqRibParser implementation

CqRibParser::CqRibParser(const boost::shared_ptr<IqRibRequestHandler>& requestHandler)
	: m_lex(),
	m_requestHandler(requestHandler),
	m_floatArrayPool(),
	m_intArrayPool(),
	m_stringArrayPool()
{ }

bool CqRibParser::parseNextRequest()
{
	// Mark array pools as free to use.
	m_floatArrayPool.markUnused();
	m_intArrayPool.markUnused();
	m_stringArrayPool.markUnused();
	CqRibToken requestTok = m_lex.get();
	bool hadRequest = true;
	try
	{
		// Get the next token, and make sure it's a request token.
		switch(requestTok.type())
		{
			case CqRibToken::REQUEST:
				// Invoke the request handler.
				m_requestHandler->handleRequest(requestTok.stringVal(), *this);
				break;
			case CqRibToken::ENDOFFILE:
				return false;
			default:
				hadRequest = false;
				tokenError("request", requestTok);
				break;
		}
	}
	catch(XqParseError& e)
	{
		// Save the error position
		SqRibPos errorPos = m_lex.pos();
		// Recover from the error by reading and discarding tokens from the RIB
		// stream up until the next request, as per the RISpec.
		CqRibToken::EqType nextType = m_lex.peek().type();
		while(nextType != CqRibToken::REQUEST && nextType != CqRibToken::ENDOFFILE)
		{
			m_lex.get();
			nextType = m_lex.peek().type();
		}
		// Add information on the location (file,line etc) of the problem to
		// the exception message and rethrow.
		std::ostringstream message;
		message << "Parse error at " << errorPos;
		if(hadRequest)
			message << " while reading " << requestTok.stringVal();
		message << ": " << e.what();
		AQSIS_THROW_XQERROR(XqParseError, e.code(), message.str());
	}
	return true;
}

void CqRibParser::pushInput(std::istream& inStream, const std::string& streamName,
		const TqCommentCallback& callback)
{
	m_lex.pushInput(inStream, streamName, callback);
}

void CqRibParser::popInput()
{
	m_lex.popInput();
}

SqRibPos CqRibParser::streamPos()
{
	return m_lex.pos();
}

TqInt CqRibParser::getInt()
{
	CqRibToken tok = m_lex.get();
	if(tok.type() != CqRibToken::INTEGER)
		tokenError("integer", tok);
	return tok.intVal();
}

TqFloat CqRibParser::getFloat()
{
	CqRibToken tok = m_lex.get();
	switch(tok.type())
	{
		case CqRibToken::INTEGER:
			return tok.intVal();
		case CqRibToken::FLOAT:
			return tok.floatVal();
		default:
			tokenError("float", tok);
			return 0;
	}
}

std::string CqRibParser::getString()
{
	CqRibToken tok = m_lex.get();
	if(tok.type() != CqRibToken::STRING)
		tokenError("string", tok);
	return tok.stringVal();
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
	CqRibToken tok = m_lex.get();
	if(tok.type() != CqRibToken::ARRAY_BEGIN)
		tokenError("integer array", tok);

	TqIntArray& buf = m_intArrayPool.getBuf();
	bool parsing = true;
	while(parsing)
	{
		CqRibToken tok = m_lex.get();
		switch(tok.type())
		{
			case CqRibToken::INTEGER:
				buf.push_back(tok.intVal());
				break;
			case CqRibToken::ARRAY_END:
				parsing = false;
				break;
			default:
				tokenError("integer array element", tok);
				break;
		}
	}
	return buf;
}

const CqRibParser::TqFloatArray& CqRibParser::getFloatArray(TqInt length)
{
	TqFloatArray& buf = m_floatArrayPool.getBuf();
	if(m_lex.peek().type() == CqRibToken::ARRAY_BEGIN)
	{
		// Read an array in [ num1 num2 ... num_n ] format

		m_lex.get(); // consume '['
		bool parsing = true;
		while(parsing)
		{
			CqRibToken tok = m_lex.get();
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
					tokenError("float array element", tok);
					break;
			}
		}

		if(length >= 0 && static_cast<TqInt>(buf.size()) != length)
		{
			AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
				"expected " << length << " float array componenets, got "
				<< buf.size());
		}
	}
	else if(length >= 0)
	{
		// Read an array in  num1 num2 ... num_n  format (ie, without the usual
		// array delimiters).
		for(TqInt i = 0; i < length; ++i)
			buf.push_back(getFloat());
	}
	else
	{
		tokenError("float array", m_lex.get());
	}
	return buf;
}

const CqRibParser::TqStringArray& CqRibParser::getStringArray()
{
	CqRibToken tok = m_lex.get();
	if(tok.type() != CqRibToken::ARRAY_BEGIN)
		tokenError("string array", tok);

	TqStringArray& buf = m_stringArrayPool.getBuf();
	bool parsing = true;
	while(parsing)
	{
		CqRibToken tok = m_lex.get();
		switch(tok.type())
		{
			case CqRibToken::STRING:
				buf.push_back(tok.stringVal());
				break;
			case CqRibToken::ARRAY_END:
				parsing = false;
				break;
			default:
				tokenError("string array element", tok);
				break;
		}
	}
	return buf;
}

void CqRibParser::getParamList(IqRibParamListHandler& paramHandler)
{
	while(true)
	{
		switch(m_lex.peek().type())
		{
			case CqRibToken::REQUEST:
			case CqRibToken::ENDOFFILE:
				// If we get to the next request or end of file, return since
				// we're done parsing the parameter list
				return;
			case CqRibToken::STRING:
				break;
			default:
				tokenError("parameter list token", m_lex.get());
		}
		// Note: we NEED to copy paramName into a temporary variable here.
		// The current token is returned by reference from the lexer, and will
		// be overwritten due to the user calling any of the parser getter
		// functions inside readParameter().
		std::string paramName = m_lex.get().stringVal();
		paramHandler.readParameter(paramName, *this);
	}
}

IqRibParser::EqRibToken CqRibParser::peekNextType()
{
	switch(m_lex.peek().type())
	{
		case CqRibToken::ARRAY_BEGIN:
			return Tok_Array;
		case CqRibToken::STRING:
			return Tok_String;
		case CqRibToken::INTEGER:
			return Tok_Int;
		case CqRibToken::FLOAT:
			return Tok_Float;
		default:
			return Tok_RequestEnd;
	}
}

const CqRibParser::TqIntArray& CqRibParser::getIntParam()
{
	if(m_lex.peek().type() == CqRibToken::INTEGER)
	{
		TqIntArray& buf = m_intArrayPool.getBuf();
		buf.push_back(m_lex.get().intVal());
		return buf;
	}
	return getIntArray();
}

const CqRibParser::TqFloatArray& CqRibParser::getFloatParam()
{
	switch(m_lex.peek().type())
	{
		case CqRibToken::INTEGER:
			{
				TqFloatArray& buf = m_floatArrayPool.getBuf();
				buf.push_back(m_lex.get().intVal());
				return buf;
			}
		case CqRibToken::FLOAT:
			{
				TqFloatArray& buf = m_floatArrayPool.getBuf();
				buf.push_back(m_lex.get().floatVal());
				return buf;
			}
		default:
			return getFloatArray();
	}
}

const CqRibParser::TqStringArray& CqRibParser::getStringParam()
{
	if(m_lex.peek().type() == CqRibToken::STRING)
	{
		// special case where next token is a single string.
		TqStringArray& buf = m_stringArrayPool.getBuf();
		buf.push_back(m_lex.get().stringVal());
		return buf;
	}
	return getStringArray();
}


/** \brief Throw an error from encountering an unexpected token.
 *
 * An error string is generated with the form:
 *
 * expected <expected> before <token_description>
 *
 * where <expected> is a string describing the expected token and
 * <token_description> is generated from the bad token provided.
 *
 * \param expected - string describing the expected token
 * \param badTok - the problematic token which was actually obtained.
 */
void CqRibParser::tokenError(const char* expected, const CqRibToken& badTok)
{
	std::ostringstream msg;

	msg << "expected " << expected << " before ";
	switch(badTok.type())
	{
		case CqRibToken::ARRAY_BEGIN:
			msg << "'['";
			break;
		case CqRibToken::ARRAY_END:
			msg << "']'";
			break;
		case CqRibToken::ENDOFFILE:
			msg << "end of file";
			// Put ENDOFFILE back into the input, since not doing so may cause
			// problems for streams which only provide a single ENDOFFILE
			// token before blocking (eg, ProcRunProgram pipes).
			m_lex.unget();
			break;
		case CqRibToken::INTEGER:
			msg << "integer [= " << badTok.intVal() << "]";
			break;
		case CqRibToken::FLOAT:
			msg << "float [= " << badTok.floatVal() << "]";
			break;
		case CqRibToken::STRING:
			msg << "string [= \"" << badTok.stringVal() << "\"]";
			break;
		case CqRibToken::REQUEST:
			msg << "request [= " << badTok.stringVal() << "]";
			// For unexpected REQUEST tokens we back up by one token so that
			// the next call to parseNextRequest() can start afresh with the
			// new request.
			m_lex.unget();
			break;
		case CqRibToken::ERROR:
			msg << "bad token [" << badTok.stringVal() << "]";
			break;
	}

	AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax, msg.str());
}

} // namespace Aqsis
