// -*- C++ -*-
// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
    \brief Binary RIB decoding class and gzip file inflation.
    \author Lionel J. Lacour (intuition01@online.fr)
*/

#ifndef RIB_BINARY_DECODER_H
#define RIB_BINARY_DECODER_H
#include <string>
#include <vector>
#include <stdio.h>
#include <zlib.h>
#include "aqsis.h"

namespace librib
{

class CqRibBinaryDecoder
{
	private:
		FILE *file; // File to read from
		z_stream zstrm; // zlib strm description
		int zerr; // zlib error state
		bool is_gzip; // Is this a gzip'd stream
		int zbuffersize; // Buffer size
		Bytef *zin, *zout; // Data buffers
		int zavailable; // Available output bytes
		Bytef *zcurrent; // Location in available output

		std::vector<TqChar> cv;

		std::string ritab[ 256 ];
		std::vector<std::string> stringtab;

		// Convert To Signed Integer
		TqInt ctsi( TqChar );
		TqInt ctsi( TqChar, TqChar );
		TqInt ctsi( TqChar, TqChar, TqChar );
		TqInt ctsi( TqChar, TqChar, TqChar, TqChar );

		// Convert To unsigned Integer
		TqUint ctui( TqChar );
		TqUint ctui( TqChar, TqChar );
		TqUint ctui( TqChar, TqChar, TqChar );
		TqUint ctui( TqChar, TqChar, TqChar, TqChar );

		// Get Char
		void gc( TqChar & );

		// Send N Characters
		void snc( TqUint, std::string & );

		void sendFloat( std::string & );
		void sendDouble( std::string & );

		void readString( TqChar, std::string & );

		// Decode Next Char
		void getNext();
		TqInt writeToBuffer( TqPchar buffer, TqUint size );

		TqBool eof_flag;
		TqBool fail_flag;

		void initZlib( int buffersize = 0 );
		CqRibBinaryDecoder( CqRibBinaryDecoder const &)
		{}
		CqRibBinaryDecoder const &operator=( CqRibBinaryDecoder const & )
		{
			return * this;
		}

	public:
		CqRibBinaryDecoder( std::string, int buffersize = 16384 );
		CqRibBinaryDecoder( FILE *, int buffersize = 16384 );
		~CqRibBinaryDecoder();

		TqInt read( TqPchar buffer, TqUint size );
		TqBool eof()
		{
			return eof_flag;
		};
		TqBool fail()
		{
			return fail_flag;
		};

};

} // namespace librib

#endif

