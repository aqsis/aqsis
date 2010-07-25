// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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
 *
 * \brief Compatibility interface to Pixar's sloarg shader argument library.
 * \author Matthias Baas
 * \author Malcolm Humphreys
 */

#include "aqsis/ri/slo.h"
#include <aqsis/ri/ri.h> // RIE_* error codes

#include <string>
#include <map>

#ifdef AQSIS_COMPILER_MSVC6
#pragma warning (disable : 4786)
#endif //AQSIS_COMPILER_MSVC6

// This is defined in slx.cpp
extern RtInt SlxLastError;

// Storage of SLO_VISSYMDEF structs
static std::map< std::string, SLO_VISSYMDEF > slovissymdef_data;
static std::map< int, std::string > slovissymdef_namemap;

/** Init the SLO_VISSYMDEF struct */
void initParamStruct ( SLO_VISSYMDEF* info )
{
	
	info->svd_name = "";
	info->svd_type = SLO_TYPE_UNKNOWN;
	info->svd_storage = SLO_STOR_UNKNOWN;
	info->svd_detail = SLO_DETAIL_UNKNOWN;
	info->svd_spacename = "";
	info->svd_default.scalarval = 0x0;
	info->svd_valisvalid = 0;
	info->svd_arraylen = 0;
	return;
}

/** Convert a SLX_VISSYMDEF to a SLO_VISSYMDEF struct */
void convertVISSYMDEFStruct ( SLX_VISSYMDEF* slxdef, SLO_VISSYMDEF* slodef )
{
	
	slodef->svd_name = slxdef->svd_name;
	slodef->svd_type = SLO_TYPE(slxdef->svd_type);
	slodef->svd_storage = SLO_STORAGE(slxdef->svd_storage);
	slodef->svd_detail = SLO_DETAIL(slxdef->svd_detail);
	slodef->svd_spacename = slxdef->svd_spacename;
	slodef->svd_valisvalid = 1;
	slodef->svd_arraylen = slxdef->svd_arraylen;
	
	// if array
	if (!(slxdef->svd_arraylen > 0 || slxdef->svd_default.scalarval == 0x0))
	{
		switch(slodef->svd_type)
		{
			case SLO_TYPE_POINT:
			case SLO_TYPE_COLOR:
			case SLO_TYPE_VECTOR:
			case SLO_TYPE_NORMAL:
				slodef->svd_default.pointval = (SLO_POINT*)slxdef->svd_default.pointval;
				break;
			case SLO_TYPE_SCALAR:
				slodef->svd_default.scalarval = slxdef->svd_default.scalarval;
				break;
			case SLO_TYPE_STRING:
				slodef->svd_default.stringval = *slxdef->svd_default.stringval;
				break;
			case SLO_TYPE_MATRIX:
				slodef->svd_default.matrixval = (SLO_MATRIX)slxdef->svd_default.matrixval;
				break;
		}
	} else {
		// SLX doesn't support array types
	}
	return;
}

/** Set colon-delimited search path used to locate compiled shaders */
void Slo_SetPath ( char* path )
{
	SLX_SetPath(path);
}

/** Attempt to locate and read the specified compiled shader */
int Slo_SetShader ( char* name )
{
	return SLX_SetShader(name);
}

/** Returns the name of the current shader */
const char* Slo_GetName ( void )
{
	return SLX_GetName();
}

/** Return type of the current shader, enumerated in SLO_TYPE */
SLO_TYPE Slo_GetType ( void )
{
	return (SLO_TYPE)SLX_GetType();
}

/** not-supported-atm **/
int Slo_HasMethod( const char* i_name )
{
	return 0;
}

/** not-supported-atm **/
const char* const* Slo_GetMethodNames()
{
	return NULL;
}

/** Return the number of arguments accepted by the current shader */
int Slo_GetNArgs ( void )
{
	return SLX_GetNArgs();
}

/** Return pointer to shader symbol definition for argument specified by symbol ID */
SLO_VISSYMDEF* Slo_GetArgById ( int id )
{
	
	// check to see if we have converted this yet
	if (slovissymdef_namemap.find(id) == slovissymdef_namemap.end())
	{
		
		SLX_VISSYMDEF* slxdef = SLX_GetArgById(id - 1); // SLX_* is 0 based
		std::string param_name(slxdef->svd_name);
		
		// catch the case where the namemap might be out of sync because of
		// other calls to Slo_GetArgByName which can't update the namemap
		if (slovissymdef_data.find(param_name) != slovissymdef_data.end())
		{
			slovissymdef_namemap[id] = param_name;
			return &slovissymdef_data[slovissymdef_namemap[id]];
		}
		
		// convert SLX_VISSYMDEF to SLO_VISSYMDEF
		SLO_VISSYMDEF slodef;
		initParamStruct(&slodef);
		if (SlxLastError == RIE_NOERROR) {
			convertVISSYMDEFStruct (slxdef, &slodef);
		} else {
			return NULL;
		}
		
		slovissymdef_data[param_name] = slodef;
		slovissymdef_namemap[id] = param_name;
	}
	
	return &slovissymdef_data[slovissymdef_namemap[id]];
}

/** Return pointer to shader symbol definition for argument specified by symbol name */
SLO_VISSYMDEF* Slo_GetArgByName ( char* name )
{
	std::string param_name(name);
	if (slovissymdef_data.find(std::string(name)) == slovissymdef_data.end())
	{
		
		SLX_VISSYMDEF* slxdef = SLX_GetArgByName(name);
		param_name = slxdef->svd_name;
		
		// convert SLX_VISSYMDEF to SLO_VISSYMDEF
		SLO_VISSYMDEF slodef;
		initParamStruct(&slodef);
		if (SlxLastError == RIE_NOERROR)
		{
			convertVISSYMDEFStruct (slxdef, &slodef);
		}
		else
		{
			return NULL;
		}
		
		slovissymdef_data[param_name] = slodef;
		// slovissymdef_namemap becomes out of sync here as we don't have an id
		// for this param, Slo_GetArgById will catch this
	}
	
	return &slovissymdef_data[param_name];
}

/** Return pointer to SLX_VISSYMDEF struct for a single element of an array */
SLO_VISSYMDEF* Slo_GetArrayArgElement( SLO_VISSYMDEF* array, int index )
{
	// The SLX_* version is not implemented yet
	return NULL;
}

/** Release storage used internally for current shader. */
void Slo_EndShader ( void )
{
	// clear the maps
	slovissymdef_data.clear();
	slovissymdef_namemap.clear();
	SLX_EndShader();
}

/** Return ASCII representation of SLX_TYPE */
char* Slo_TypetoStr ( SLO_TYPE type )
{
	return SLX_TypetoStr((SLX_TYPE)type);
}

/** Return ASCII representation of SLX_STORAGE */
char* Slo_StortoStr ( SLO_STORAGE storage )
{
	return SLX_StortoStr((SLX_STORAGE)storage);
}

/** Return ASCII representation of SLX_DETAIL */
char* Slo_DetailtoStr ( SLO_DETAIL detail )
{
	return SLX_DetailtoStr((SLX_DETAIL)detail);
}
