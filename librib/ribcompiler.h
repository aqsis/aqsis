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
		\brief Declares CqRIBCompiler class responsible for processing RIB files.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef RIBCOMPILER_H_INCLUDED
#define RIBCOMPILER_H_INCLUDED 1

#include	<vector>

#include	"aqsis.h"

#include	"iribcompiler.h"

#include	"parser.h"
#include	"scanner.h"

START_NAMESPACE(Aqsis)

//----------------------------------------------------------------------
/** \class CqRIBCompiler
 * Class encapsulating an ascii RIB compiler.
 */

class CqRIBCompiler : public RIBParser, public IqRIBCompiler
{
	public:
			CqRIBCompiler()			{}							
	virtual	~CqRIBCompiler()	{}

	// Overridden from IsRIBCompiler

						/** Set the file to use for parsing.
						 * \param file An open valid CqFile object.
						 */				
			void	SetFile(std::istream& file, const char* name)	{m_Scanner.SetFile(file,name);}
						/** Parse the RIB file.
						 * \return Integer error code.
						 */
			TqInt	Parse()					{return(yyparse());}
						/** Destroy this RIBCompiler, called through the interface to clean up.
						 */
			void	Destroy()				{delete(this);}
	
	private:
		// Local functions
			TqInt	yyparse();
			TqInt	yylex();
			void	yyerror(const TqChar *m);
			void	yydebugout(const TqChar *m);

		// Functions overridden from RIBParser
		void	ExpectRequest()			{m_Scanner.ExpectRequest();}
		void	ExpectParams()			{m_Scanner.ExpectParams();}

			RIBScanner	m_Scanner;		///< Instance of the flex scanner.
};

//-----------------------------------------------------------------------


END_NAMESPACE(Aqsis)

#endif	// !RIBCOMPILER_H_INCLUDED
