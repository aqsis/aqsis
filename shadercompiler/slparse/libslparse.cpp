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
		\brief Implements the entrypoint for the shader compiler.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include "libslparse.h"
#include "parsenode.h"

#include "logging.h"

extern int yyparse();
#ifdef	YYDEBUG
extern int yydebug;
#endif

START_NAMESPACE( Aqsis )



extern CqParseNode* ParseTreePointer;
extern void TypeCheck();
extern void Optimise();
extern void InitStandardNamespace();


std::istream* ParseInputStream = &std::cin;
CqString ParseStreamName = "stdin";
std::ostream* ParseErrorStream = &std::cerr;
TqInt ParseLineNumber;
TqBool ParseSucceeded = true;

TqBool Parse( std::istream& InputStream, const CqString StreamName, std::ostream& ErrorStream )
{
	ParseInputStream = &InputStream;
	ParseStreamName = StreamName;
	ParseErrorStream = &ErrorStream;
	ParseLineNumber = 1;
	ParseSucceeded = true;

	InitStandardNamespace();

	try
	{
#ifdef	YYDEBUG
		yydebug = 1;
#endif

		yyparse();
		TypeCheck();
	}
	catch(CqString strError)
	{
		( *ParseErrorStream ) << error << strError.c_str() << std::endl;
		( *ParseErrorStream ) << error << "Shader not compiled" << std::endl;
		ParseSucceeded = false;
		return( false );
	}
	Optimise();

	std::vector<CqVarDef>::iterator iv;
	for ( iv = gLocalVars.begin(); iv != gLocalVars.end(); iv++ )
		if ( iv->pDefValue() )
			iv->pDefValue() ->Optimise();

	return ParseSucceeded;
}

void ResetParser()
{
	ParseInputStream = &std::cin;
	ParseStreamName = "stdin";
	ParseErrorStream = &std::cerr;
	ParseLineNumber = 1;
	ParseSucceeded = true;
}


IqParseNode* GetParseTree()
{
	return ( ParseTreePointer );
}

END_NAMESPACE( Aqsis )

//-------------------------------------------------------------
