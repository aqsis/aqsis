// -*- C++ -*-
// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
 *  \brief A class to parse RiDeclare and inline parameter list definitions.
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */

#ifndef RI2RIB_INLINEPARSE_H
#define RI2RIB_INLINEPARSE_H 1

#include <stdio.h>
#include <ctype.h>
#include <string>

#include "aqsis.h"
#include "ri.h"
#include "dictionary.h"
#include "error.h"

START_NAMESPACE( libri2rib )

class CqInlineParse
{
	private:
		TqUint number_of_words;
		std::string word[ 7 ];

		bool inline_def;
		EqTokenClass tc;
		EqTokenType tt;
		TqUint size;
		std::string identifier;


		bool is_class ( const std::string &str );
		bool is_type ( const std::string &str );
		bool is_int ( const std::string &str );

		EqTokenClass get_class ( const std::string &str );
		EqTokenType get_type ( const std::string &str );
		TqUint get_size ( const std::string &str );

		void check_syntax ();
		void lc( std::string & );

	public:
		CqInlineParse()
		{}
		~CqInlineParse()
		{}
		void parse ( std::string &str );

		bool isInline()
		{
			return inline_def;
		}
		EqTokenClass getClass()
		{
			return tc;
		}
		EqTokenType getType()
		{
			return tt;
		}
		TqUint getQuantity()
		{
			return size;
		}
		std::string getIdentifier()
		{
			return identifier;
		}
};

END_NAMESPACE( libri2rib )
#endif
