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


/** \file	mtable.h
		\brief The message table
		\author Matthäus G. Chajdas (Matthaeus@darkside-conflict.net)
*/

/*
	*	RI_ERROR			0		RiError: Unknown error
	*	BASIC_ERROR			0		BasicError: Unkown error
	*	SHADER_ERROR		0		ShaderError: Unknown error
*/


//? Is .h included already?
#ifndef MTABLE_H_INCLUDED
#define MTABLE_H_INCLUDED 1

#pragma warning (disable : 4786)

#include <map>
#include <string>
#include "aqsis.h"

START_NAMESPACE( Aqsis )

typedef std::map< int, std::string > i_s;

class CqMessageTable
{
	public:
		CqMessageTable();

		const char* getError( int table, int error_id );
		void insert( int table, i_s::value_type data );
		
		int getErrorCount()
		{
			return m_errorCount;
		}

		int getCacheHits()
		{
			return m_cacheHits;
		}
		
	private:
		// Stats
		int m_errorCount;
		int m_cacheHits;

		// Cache
		int m_cTable;
		int m_cError;
		std::string m_cErrorText;

		// Pointers
		i_s* m_pTable;
		i_s::iterator* m_pIt;
		
		// Tables
		i_s m_Be;
		i_s::iterator m_Be_i;

		i_s m_Ri;
		i_s::iterator m_Ri_i;

		i_s m_Sh;
		i_s::iterator m_Sh_i;

};

END_NAMESPACE( Aqsis )

#endif

