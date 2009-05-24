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
 * \brief Compatibility layer implementing the Pixar sloarg shader argument interface.
 *
 * \author Matthias Baas
 */

#include <aqsis/ri/slo.h>

#include <stdlib.h>
#include <string.h>

#include <aqsis/ri/ri.h> // For RtInt and RIE_* error codes.

#ifdef AQSIS_COMPILER_MSVC6
#pragma warning (disable : 4786)
#endif //AQSIS_COMPILER_MSVC6

// This is defined in slx.cpp
extern RtInt SlxLastError;

/// Just the shader name (no path, no ext).
static char *currentShaderName = NULL;

// The following four variables represent a lookup table to
// map SLX_VISSYMDEF objects to SLO_VISSYMDEF objects.
static int sloShadersArgsNumItems = 0;
static int sloShadersArgsMaxNumItems = 0;
static SLX_VISSYMDEF* * sloShadersArgsKeys = NULL;
static SLO_VISSYMDEF* * sloShadersArgsValues = NULL;

/**
 * Initialise the shader name from a file name.
 * 
 * The function simply removes any path information and the extension.
 * Returns an RIE_* error code (RIE_NOERROR on success).
 * fileName must not be NULL.
 */
static RtInt InitShaderName( char * fileName )
{
	RtInt result;
	char * p1;
	char * p2;
	char * name;
	int n;
	
	result = RIE_NOERROR;
	
	if ( currentShaderName != NULL )
	{
		free( currentShaderName );
	}

	// Determine the start of the file name...
	p1 = strrchr( fileName, '/' );
	p2 = strrchr( fileName, '\\' );
	name = (p1>p2)? p1 : p2;
	if ( name == NULL )
	{
		name = fileName;
	}
	else
	{
		name += 1;
	}
	
	// name now points to the file name part
	
	// Search for the suffix and determine the length of the file name without suffix
	p1 = strrchr( name, '.' );
	if ( p1 == NULL )
	{
		n = strlen( name );
	}
	else
	{
		n = p1 - name;
	}
	
	currentShaderName = (char*) malloc( n + 1 );
	if ( currentShaderName == NULL )
	{
		result = RIE_NOMEM;
	}
	else
	{
		strncpy( currentShaderName, name, n );
		currentShaderName[n] = 0;
	}

	return result;
}

/**
 * Extend the capacity of the SLX -> SLO lookup table.
 * 
 * Extends the lookup table by doubling its capacity.
 * 
 * \return Returns 0 on success, -1 on failure (sets the global SlxLastError value).
 */
static int ExtendSLOLut()
{
	int newSize;
	SLX_VISSYMDEF* * newKeys;
	SLO_VISSYMDEF* * newValues;

	SlxLastError = RIE_NOERROR;

	// Determine the new table size...
	if ( sloShadersArgsMaxNumItems==0 )
	{
		newSize = 1;
	}
	else
	{
		newSize = 2*sloShadersArgsMaxNumItems;
	}
	
	// Allocate a new table...
	newKeys = (SLX_VISSYMDEF**) malloc( newSize*sizeof(SLX_VISSYMDEF*) );
	if ( newKeys == NULL )
	{
		SlxLastError = RIE_NOMEM;
	}
	
	if ( SlxLastError == RIE_NOERROR )
	{
		newValues = (SLO_VISSYMDEF**) malloc( newSize*sizeof(SLO_VISSYMDEF*) );
		if ( newValues == NULL )
		{
			SlxLastError = RIE_NOMEM;
		}
	}
	
	// Copy the old table data into the new table and delete the old table...
	if ( SlxLastError == RIE_NOERROR )
	{
		if ( sloShadersArgsKeys != NULL )
		{
			memcpy( newKeys, sloShadersArgsKeys, sloShadersArgsNumItems*sizeof(SLX_VISSYMDEF*) );
			free( sloShadersArgsKeys );
		}
		if ( sloShadersArgsValues != NULL )
		{
			memcpy( newValues, sloShadersArgsValues, sloShadersArgsNumItems*sizeof(SLO_VISSYMDEF*) );
			free( sloShadersArgsValues );
		}
		
		sloShadersArgsKeys = newKeys;
		sloShadersArgsValues = newValues;
		sloShadersArgsMaxNumItems = newSize;
	}

	return SlxLastError==RIE_NOERROR? 0 : -1;
}

/**
 * Insert a new entry to the SLX -> SLO lookup table.
 * 
 * The SLO struct is created internally from the input SLX struct and
 * will be added at the end of the table.
 * 
 * \pre  key must not already be present in the lookup table
 * \pre  key must not be NULL
 * \param key A SLX_VISSYMDEF structure
 * \return Returns the newly created SLO struct or NULL on error (sets the global error flag).
 */
static SLO_VISSYMDEF * InsertSLOSymDef(SLX_VISSYMDEF * key)
{
	SLO_VISSYMDEF * value;

	SlxLastError = RIE_NOERROR;
	
	value = ( SLO_VISSYMDEF * ) malloc( sizeof( SLO_VISSYMDEF ) );
	if ( value == NULL )
	{
		SlxLastError = RIE_NOMEM;
	}
	
	// Initialise the SLO struct from the SLX struct and add it to the lookup table
	// (all pointers are still owned by the SLX struct!)
	if ( SlxLastError == RIE_NOERROR )
	{
		value->svd_name = key->svd_name;
		value->svd_type = SLO_TYPE(key->svd_type);
		value->svd_storage = SLO_STORAGE(key->svd_storage);
		value->svd_detail = SLO_DETAIL(key->svd_detail);
		value->svd_spacename = key->svd_spacename;
		value->svd_arraylen = key->svd_arraylen;
		value->svd_default.scalarval = value->svd_default.scalarval;
		if ( value->svd_type == SLO_TYPE_STRING )
		{
			// Dereference the stringval
			value->svd_default.stringval = *(key->svd_default.stringval);
		}

		// Do we need to extend the table?
		if ( sloShadersArgsNumItems == sloShadersArgsMaxNumItems )
		{
			if ( ExtendSLOLut() == -1 )
			{
				// The error indicator is already set, so just release the SLO struct
				free( value );
				value = NULL;
			}
		}
		
		// Store the key/value pair...
		if ( SlxLastError == RIE_NOERROR )
		{
			sloShadersArgsKeys[sloShadersArgsNumItems] = key;
			sloShadersArgsValues[sloShadersArgsNumItems] = value;
			sloShadersArgsNumItems++;
		}
	}
	
	return value;
}

/**
 * Look up a SLO_VISSYMDEF structure from a SLX_VISSYMDEF struct.
 * 
 * The function returns a SLO_VISSYMDEF struct that corresponds
 * to the input SLX_VISSYMDEF struct. If called twice with the same
 * input, the function will also return the same output struct.
 * If called the first time with a particular SLX struct,
 * the function creates a new SLO struct.
 * 
 * \param key  A SLX_VISSYMDEF struct
 * \return Returns the SLO_VISSYMDEF struct associated with key
 *         or NULL if there was an error. The function also sets
 *         the global error flag.
 */
static SLO_VISSYMDEF * LookUpSLOSymDef(SLX_VISSYMDEF * key)
{
	int i;
	
	SlxLastError = RIE_NOERROR;
	
	// Check if we already have the struct...
	for( i = 0; i<sloShadersArgsNumItems; i++ )
	{
		if ( sloShadersArgsKeys[i] == key )
			return sloShadersArgsValues[i];
	}
	
	return InsertSLOSymDef( key );
}

/**
 * Delete the SLX -> SLO lookup table.
 * 
 * This has to be called in EndShader().
 */
static void DeleteSLOLut()
{
	int i;
	
	if ( sloShadersArgsKeys != NULL )
	{
		free( sloShadersArgsKeys );
		sloShadersArgsKeys = NULL;
	}
	if ( sloShadersArgsValues != NULL )
	{
		for( i=0; i<sloShadersArgsNumItems; i++)
		{
			free( sloShadersArgsValues[i] );
			sloShadersArgsValues[i] = NULL;
		}
		free (sloShadersArgsValues );
		sloShadersArgsValues = NULL;
	}
	sloShadersArgsNumItems = 0;
	sloShadersArgsMaxNumItems = 0;
}


/**
 * Set colon-delimited search path used to locate compiled shaders.
 * 
 * \param path The search paths.
 */
void Slo_SetPath ( char * path )
{
	SLX_SetPath( path );
}

/**
 * Attempt to locate and read the specified compiled shader.
 * 
 * Search the current shader search paths for the specified shader
 * and make the shader current. The name may include a file suffix
 * or it can just be the base shader name.
 * 
 * \param name  Shader name (with or without suffix)
 * \return Returns 0 on sucess, -1 on error.
 */
int Slo_SetShader ( char * name )
{
	int res;
	char * slxname;
	
	// Check if there is still a shader name (just to be on the safe side)
	if ( currentShaderName != NULL )
	{
		free( currentShaderName );
		currentShaderName = NULL;
	}

	// Call the SLX version
	res = SLX_SetShader( name );
	
	// Extract the shader name from the shader file name...
	if ( res == 0 )
	{
		slxname = SLX_GetName();
		if ( slxname == NULL)
		{
			SLX_EndShader();
			res = -1;
		}
		else
		{
			InitShaderName( slxname );
		}
	}
	
	return res;
}

/**
 * Return pointer to a string containing the name of the current shader.
 * 
 * \return  Returns the name of the current shader or NULL if no shader is set.
 */
char *Slo_GetName ( void )
{
	return currentShaderName;
}

/**
 * Return type of the current shader, enumerated in SLO_TYPE.
 */
SLO_TYPE Slo_GetType ( void )
{
	return (SLO_TYPE)SLX_GetType();
}


/**
 * Return the number of arguments accepted by the current shader.
 */
int Slo_GetNArgs ( void )
{
	return SLX_GetNArgs();
}

/**
 * Return pointer to shader symbol definition for argument specified by symbol ID.
 * 
 * \param id  Argument index (1-based).
 * \return Returns a SLO_VISSYMDEF struct or NULL on error.
 */
SLO_VISSYMDEF * Slo_GetArgById ( int id )
{
	SLX_VISSYMDEF * slxdef;
	SLO_VISSYMDEF * slodef;
	
	slodef = NULL;
	slxdef = SLX_GetArgById( id-1 );
	if ( slxdef != NULL )
	{
		slodef = LookUpSLOSymDef( slxdef );
	}
	return slodef;
}

/*
 * Return pointer to shader symbol definition for argument specified by symbol name.
 * 
 * \param name  Parameter name.
 * \return Returns a SLO_VISSYMDEF struct or NULL on error.
 */
SLO_VISSYMDEF * Slo_GetArgByName ( char * name )
{
	SLX_VISSYMDEF * slxdef;
	SLO_VISSYMDEF * slodef;
	
	slodef = NULL;
	slxdef = SLX_GetArgByName( name );
	if ( slxdef != NULL )
	{
		slodef = LookUpSLOSymDef( slxdef );
	}
	return slodef;
}

SLO_VISSYMDEF *Slo_GetArrayArgElement( SLO_VISSYMDEF * array, int index )
{
	// The SLX_ version is not implemented yet
	
	return NULL;
}


/**
 * Release storage used internally for current shader.
 */
void Slo_EndShader ( void )
{
	SLX_EndShader();
	
	if ( currentShaderName != NULL )
	{
		free( currentShaderName );
		currentShaderName = NULL;
	}

	// Delete the data required for the Slo part of the interface
	DeleteSLOLut();
}

/**
 * Return ASCII representation of SLX_TYPE.
 */
char *Slo_TypetoStr ( SLO_TYPE type )
{
	return SLX_TypetoStr( (SLX_TYPE)type );
}

/**
 * Return ASCII representation of SLX_STORAGE.
 */
char *Slo_StortoStr ( SLO_STORAGE storage )
{
	return SLX_StortoStr( (SLX_STORAGE)storage );
}

/**
 * Return ASCII representation of SLX_DETAIL.
 */
char *Slo_DetailtoStr ( SLO_DETAIL detail )
{
	return SLX_DetailtoStr( (SLX_DETAIL)detail );
}
