// -*- C++ -*-
// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
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
 *  \brief RIB ascii output class
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */

#ifndef RI2RIB_ASCII_H
#define RI2RIB_ASCII_H 1

#include "output.h"

namespace libri2rib {

class CqASCII : public CqOutput
{
	public:
		CqASCII( const char *name, const int fdesc,
		         const SqOptions::EqCompression comp,
		         const SqOptions::EqIndentation i, const TqInt isize );
		virtual ~CqASCII();
	protected:
		virtual void beginNesting(EqBlocks type);
		virtual void endNesting(EqBlocks type);

		virtual void printHeader();
		virtual void printIndentation();
		virtual void printRequest( const char *, EqFunctions );
		virtual void printInteger( const RtInt );
		virtual void printFloat( const RtFloat );
		virtual void printString( const std::string & );
		virtual void printSpace();
		virtual void printEOL();

		virtual void printArray ( RtInt n, RtInt *p );
		virtual void printArray ( RtInt n, RtFloat *p );
		virtual void printArray ( RtInt n, RtToken *p );
		virtual void printToken ( RtToken t );
		virtual void printCharP ( const char *c );
		virtual void print ( const char *c );

	private:
		SqOptions::EqIndentation m_Indentation;
		TqInt m_IndentSize;
		TqInt m_IndentLevel;
}
;

} // namespace libri2rib
#endif
