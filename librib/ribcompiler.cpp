// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
		\brief Implements CqRIBCompiler class responsible for processing RIB files.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<iostream>

#include	"aqsis.h"
#include	"ribcompiler.h"


START_NAMESPACE(Aqsis)

//----------------------------------------------------------------------
/** Call through to the RIB parser to parse the RIB stream.
 */

int CqRIBCompiler::yyparse()
{
	int res=RIBParser::yyparse();
	m_Scanner.yy_delete_buffer(m_Scanner.YY_CURRENT_BUFFER);
	return(res);
}



//----------------------------------------------------------------------
/** Call the lexical analyser to scan the input stream.
 */

int CqRIBCompiler::yylex()
{
	yylloc.first_line=m_Scanner.m_Line;
	int token=m_Scanner.yylex(&yylval,&yylloc);
	yylloc.last_line=m_Scanner.m_Line;
	yylloc.text=(char *)m_Scanner.yytext;
	return(token);
}


//----------------------------------------------------------------------
/** Output the text describing a parse error.
 */

void CqRIBCompiler::yyerror(const char *m)
{
	CqString String(m);
	String+=" : ";
	String+=yylloc.last_line;
	String+=" : ";
	String+=CqString(m_Scanner.m_pfile->strRealName());
	std::cerr << String.c_str() << std::endl;	
}


//----------------------------------------------------------------------
/** Output the debig text.
 */

void CqRIBCompiler::yydebugout(const char *m)
{
	fprintf(stderr,m);	
}


IsRIBCompiler* IsRIBCompiler::Create()
{
	return(new CqRIBCompiler);
}

END_NAMESPACE(Aqsis)

//---------------------------------------------------------------------
