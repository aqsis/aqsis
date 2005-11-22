// -*- C++ -*-
// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
 *  \brief Binary encoding output class
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */

#ifndef RI2RIB_BINARY_H
#define RI2RIB_BINARY_H 1

#include <list>
#include "output.h"

START_NAMESPACE( libri2rib )

class CqBinary : public CqOutput
{
private:
    void intToChar( RtInt n, TqChar &b1, TqChar &b2, TqChar &b3, TqChar &b4 );
    void floatToChar( RtFloat f, TqChar &b1, TqChar &b2, TqChar &b3, TqChar &b4 );
    void addString( std::string &, TqBool &, TqUint & );
    void encodeString( const char * );

    TqBool m_aRequest[ LAST_Function ];
    std::list<std::string> m_aStrings;

protected:
    void printHeader();
    void printRequest( const char *, EqFunctions );
    void printInteger( const RtInt );
    void printFloat( const RtFloat );
    void printString( std::string & );
    void printSpace()
    {}
    void printEOL()
    {}

    void printArray ( RtInt n, RtInt *p );
    void printArray ( RtInt n, RtFloat *p );
    void printArray ( RtInt n, RtToken *p );
    void printToken ( RtToken t );
    void printCharP ( const char *c );
    void print ( const char *c );

public:
    CqBinary( const char *, int fdesc,
              SqOptions::EqCompression,
              SqOptions::EqIndentation, TqInt isize );
    ~CqBinary()
    {}
}
;

END_NAMESPACE( libri2rib )
#endif
