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
		\brief The message table itself
		\author Matthäus G. Chajdas (Matthaeus@darkside-conflict.net)
*/

#include "mtable.h"
#include "log.h"
#include "aqsis.h"

START_NAMESPACE( Aqsis )

//------------------------------------------------------------------------------
/**
 *	Constructor
 *	Fills the tables with data
 *
 */
CqMessageTable::CqMessageTable()
{
	m_cTable = -1;
	m_cError = -1;
	m_cacheHits = 0;
	m_errorCount = 0;

	// Basic errors
	m_Be[0] = "BasicError: Unknown error";

	// RI Error
	m_Ri[0] = "RiError: Unknown error";
	m_Ri[CqLog::RI_COLOR_SAMPLES_INVALID] = "RiColorSamples not supported, defaults to 3";
	m_Ri[CqLog::RI_RELATIVE_DETAIL_INVALID] = "RiRelativeDetail (invalid scaling factor)";
	m_Ri[CqLog::RI_UNKNOWN_SYMBOL] = "Unknown Symbol";
	m_Ri[CqLog::RI_AREA_LIGHT_UNSUPPORTED] = "RiAreaLightSource not supported, creating a point lightsource";
	m_Ri[CqLog::RI_INTERIOR_UNSUPPORTED] = "RiInterior shaders not supported";
	m_Ri[CqLog::RI_EXTERIOR_UNSUPPORTED] = "RiExterior shaders not supported";
	m_Ri[CqLog::RI_DETAIL_RANGE_INVALID] = "RiDetailRange (invalid levels of detail)";
	m_Ri[CqLog::RI_GEOMETRIC_APPROX_UNSUPPORTED] = "RiGeometricApproximation not supported";
	m_Ri[CqLog::RI_PERSPECTIVE_BAD_FOV] = "RiPerspective given bad fov value.";
	m_Ri[CqLog::RI_DEFORMATION_UNSUPPORTED] = "RiDeformation shaders not supported";
	m_Ri[CqLog::RI_TRANSFORM_POINTS_UNSUPPORTED] = "RiTransformPoints not supported";
	m_Ri[CqLog::RI_BLOBBY_V_UNSUPPORTED] = "RiBlobbyV not supported";
	m_Ri[CqLog::RI_CURVES_V_UNKNOWN_WRAP_MODE] = "RiCurvesV (unknown wrap mode: must be \"periodic\" or \"nonperiodic\")";
	m_Ri[CqLog::RI_CURVES_V_UNKNOWN_TYPE] = "RiCurvesV (unknown type: must be \"linear\" or \"cubic\")";
	m_Ri[CqLog::RI_PROC_DELAYED_READ_ARCHIVE_UNSUPPORTED] = "RiProcDelayedReadArchive not supported";
	m_Ri[CqLog::RI_PROC_RUN_PROGRAM_UNSUPPORTED] = "RiProcRunProgram not supported";
	m_Ri[CqLog::RI_PROC_DYNAMIC_LOAD_UNSUPPORTED] = "RiProcDynamicLoad not supported";
	m_Ri[CqLog::RI_PROCEDURAL_UNKNOWN_SUBDIV] = "RiProcedural Unknown SubdivFunc type";
	m_Ri[CqLog::RI_GEOMETRY_V_UNKNOWN] = "RiGeometryV, unknown geometry";
	m_Ri[CqLog::RI_OBJECT_BEGIN_UNSUPPORTED] = "RiObjectBegin, instances not supported";
	m_Ri[CqLog::RI_OBJECT_END_UNSUPPORTED] = "RiObjectInstance, instances not supported";
	m_Ri[CqLog::RI_MAKE_BUMP_UNSUPPORTED] = "RiMakeBump not supported";

}


CqMessageTable::~CqMessageTable()
{
	m_Be.clear();
	m_Ri.clear();
}

//------------------------------------------------------------------------------
/**
 *	Get an error message.
 *	Also called by the macro getErrorMessage()
 *
 *	@param	table	The message table to be used (*_ERROR_TABLE).
 *	@param	error_id	The message id
 *
 *	@return			The error message, or a Error/Table not found.
 */
const char* CqMessageTable::getError( int table, int error_id )
{
	m_errorCount++;
	// Return cached result if possible
	if ( ( table == m_cTable ) && ( error_id == m_cError ) )
	{
		m_cacheHits++;
		return m_cErrorText.c_str();
	}

	switch ( table )
	{
		case CqLog::BASIC_ERROR_TABLE:
		{
			m_pTable = &m_Be;
			m_pIt = &m_Be_i;
			break;
		}
		
		case CqLog::RI_ERROR_TABLE:
		{	
			m_pTable = &m_Ri;
			m_pIt = &m_Ri_i;
			break;
		}

		default:
			return "Table not found";
	}

	*m_pIt = m_pTable->find( error_id );
	if ( *m_pIt != m_pTable->end() )
	{	
		// Cache the result
		m_cError = error_id;
		m_cTable = table;
		m_cErrorText = (*m_pIt)->second;

		return m_cErrorText.c_str();
	}
	else
		return "Error not found";
}

//------------------------------------------------------------------------------
/**
 *	Insert a message.
 *
 *	@param	table	The message table to be used (*_ERROR_TABLE).
 *	@param	data	The message as a i_s::value_type
 *
 */
void CqMessageTable::insert( int table, i_s::value_type data)
{
	switch ( table )
	{
		case CqLog::BASIC_ERROR_TABLE:
		{
			m_Be.insert( data );
			break;
		}

		case CqLog::RI_ERROR_TABLE:
		{	
			m_Ri.insert( data );
			break;
		}

	}
}
  
END_NAMESPACE( Aqsis )