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
*/


//? Is .h included already?
#ifndef MTABLE_H_INCLUDED
#define MTABLE_H_INCLUDED 1

#pragma warning (disable : 4786)

#include <map>
#include <string>
#include "aqsis.h"

#define	 _qShareName	BUILD_LIBAQSISTYPES
#include "share.h"
#include "imtable.h"


START_NAMESPACE( Aqsis )

typedef std::map< int, std::string > i_s;

class _qShareC CqMessageTable	: public IqMessageTable
{
	public:
		_qShareM CqMessageTable();
		_qShareM ~CqMessageTable();

		_qShareM const char* getError( const int table, const int error_id );
		_qShareM void insert( int table, i_s::value_type data );
		
		_qShareM int getErrorCount()
		{
			return m_errorCount;
		}

		_qShareM int getCacheHits()
		{
			return m_cacheHits;
		}

		
		enum {	BASIC_ERROR_TABLE = 0,
				RI_ERROR_TABLE } EqErrorTables;

		enum {	UNKNOW_ERROR = 0,
				RI_COLOR_SAMPLES_INVALID,
				RI_RELATIVE_DETAIL_INVALID,
				RI_UNKNOWN_SYMBOL,
				RI_AREA_LIGHT_UNSUPPORTED,
				RI_INTERIOR_UNSUPPORTED,							// 5
				RI_EXTERIOR_UNSUPPORTED,
				RI_DETAIL_RANGE_INVALID,
				RI_GEOMETRIC_APPROX_UNSUPPORTED,
				RI_PERSPECTIVE_BAD_FOV,
				RI_DEFORMATION_UNSUPPORTED,							//10
				RI_TRANSFORM_POINTS_UNSUPPORTED,					
				RI_BLOBBY_V_UNSUPPORTED,
				RI_CURVES_V_UNKNOWN_WRAP_MODE,
				RI_CURVES_V_UNKNOWN_TYPE,
				RI_PROC_DELAYED_READ_ARCHIVE_UNSUPPORTED,			//15
				RI_PROC_RUN_PROGRAM_UNSUPPORTED,					
				RI_PROC_DYNAMIC_LOAD_UNSUPPORTED,
				RI_PROCEDURAL_UNKNOWN_SUBDIV,
				RI_GEOMETRY_V_UNKNOWN,
				RI_OBJECT_BEGIN_UNSUPPORTED,						//20
				RI_OBJECT_END_UNSUPPORTED,						
				RI_MAKE_BUMP_UNSUPPORTED,
				RI_ONLY_UNIFORM_OPTIONS,
				RI_ONLY_UNIFORM_ATTRIBUTES,
				RI_INVALID_SHADING_INTERPOLATION,
				RI_DEGENRATE_POLYGON,								//25
				RI_MAKE_CUBE_ENV_WRONG_SIZE,
				RI_SDS_NONMANIFOLD	} EqErrorMessages;	
		
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
};

END_NAMESPACE( Aqsis )

#endif

