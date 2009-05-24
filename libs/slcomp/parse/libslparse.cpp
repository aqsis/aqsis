// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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
		\brief Implements the entrypoint for the shader compiler.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include <aqsis/slcomp/libslparse.h>

#include <aqsis/util/logging.h>
#include <aqsis/util/exception.h>
#include "parsenode.h"
#include "vardef.h"

extern int yyparse();
#ifdef	YYDEBUG
extern int yydebug;
#endif

namespace Aqsis {



extern CqParseNode* ParseTreePointer;
extern void TypeCheck();
extern void Optimise();
extern void InitStandardNamespace();


std::istream* ParseInputStream = &std::cin;
CqString ParseStreamName = "stdin";
std::ostream* ParseErrorStream = &Aqsis::log();
TqInt ParseLineNumber;

bool Parse( std::istream& InputStream, const std::string& StreamName, std::ostream& ErrorStream )
{
	ParseInputStream = &InputStream;
	ParseStreamName = StreamName;
	ParseErrorStream = &ErrorStream;
	ParseLineNumber = 1;

	InitStandardNamespace();

	try
	{
#ifdef	YYDEBUG
		yydebug = 1;
#endif

		yyparse();
		TypeCheck();
	}
	catch(XqParseError e)
	{
		( *ParseErrorStream ) << error << e.what() << std::endl;
		( *ParseErrorStream ) << error << "Shader not compiled" << std::endl;
		return false;
	}
	catch(...)
	{
		( *ParseErrorStream ) << error << "unknown exception" << std::endl;
		( *ParseErrorStream ) << error << "Shader not compiled" << std::endl;
		return false;
	}
	Optimise();

	std::vector<CqVarDef>::iterator iv;
	for ( iv = gLocalVars.begin(); iv != gLocalVars.end(); iv++ )
		if ( iv->pDefValue() )
			iv->pDefValue() ->Optimise();

	return true;
}

void ResetParser()
{
	ParseInputStream = &std::cin;
	ParseStreamName = "stdin";
	ParseErrorStream = &Aqsis::log();
	ParseLineNumber = 1;
	/// \todo Code Review: It might be best to remove these global variables -
	// they've now become a liability since we've moved to compiling many
	// shaders in one go.  The symbol tables probably need a hard look anyway.
	gLocalVars.clear();
	gLocalFuncs.clear();
	for(TqInt i = 0; i < EnvVars_Last; ++i)
		gStandardVars[i].ResetUseCount();
}


IqParseNode* GetParseTree()
{
	return ( ParseTreePointer );
}

} // namespace Aqsis

//-------------------------------------------------------------
