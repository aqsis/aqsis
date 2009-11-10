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
		\brief Implements libslxargs, analogous to Pixar's sloarg library.
		\author Douglas Ward (dsward@vidi.com)
*/

/* TO DO:
 *  1 - Set svd_spacename for type SLX_TYPE_POINT in SLX_VISSYMDEF records.  Currently hard coded to RI_SHADER.
 *      Valid values should include RI_CURRENT, RI_SHADER, RI_EYE or RI_NDC.
 *  2 - Set svd_spacename for type SLX_TYPE_COLOR in SLX_VISSYMDEF records.  Currently hard codes to RI_RGB.
 *      Valid values should include RI_RGB, RI_RGBA, RI_RGBZ, RI_RGBAZ, RI_A, RI_Z or RI_AZ.
 */

#include <aqsis/aqsis.h>
#include <aqsis/ri/slx.h>

#ifdef	AQSIS_COMPILER_MSVC6
#pragma warning (disable : 4786)
#endif //AQSIS_COMPILER_MSVC6

#include <fstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <boost/shared_ptr.hpp>

#include <aqsis/ri/ri.h>
#include <aqsis/shadervm/ishader.h>
#include <aqsis/shadervm/ishaderdata.h>
#include <aqsis/math/color.h>

using namespace Aqsis;

#define RI_SHADER_EXTENSION ".slx"

// Global variables
RtInt SlxLastError;

static char *shaderSearchPathList = NULL;
static char *DSOPath = NULL;

static char *currentShaderSearchPath = NULL;
static char *currentShader = NULL;
static char *currentShaderFilePath = NULL;

static SLX_TYPE currentShaderType = SLX_TYPE_UNKNOWN;
static int currentShaderNArgs = 0;
static SLX_VISSYMDEF * currentShaderArgsArray = NULL;

static const char * SLX_TYPE_UNKNOWN_STR = "unknown";
static const char * SLX_TYPE_POINT_STR = "point";
static const char * SLX_TYPE_NORMAL_STR = "normal";
static const char * SLX_TYPE_VECTOR_STR = "vector";
static const char * SLX_TYPE_COLOR_STR = "color";
static const char * SLX_TYPE_SCALAR_STR = "float";
static const char * SLX_TYPE_STRING_STR = "string";
static const char * SLX_TYPE_MATRIX_STR = "matrix";
static const char * SLX_TYPE_SURFACE_STR = "surface";
static const char * SLX_TYPE_LIGHT_STR = "light";
static const char * SLX_TYPE_DISPLACEMENT_STR = "displacement";
static const char * SLX_TYPE_VOLUME_STR = "volume";
static const char * SLX_TYPE_TRANSFORMATION_STR = "transformation";
static const char * SLX_TYPE_IMAGER_STR = "imager";

static const char * SLX_STOR_UNKNOWN_STR = "unknown";
static const char * SLX_STOR_CONSTANT_STR = "constant";
static const char * SLX_STOR_VARIABLE_STR = "variable";
static const char * SLX_STOR_TEMPORARY_STR = "temporary";
static const char * SLX_STOR_PARAMETER_STR = "parameter";
static const char * SLX_STOR_OUTPUTPARAMETER_STR = "output parameter";
static const char * SLX_STOR_GSTATE_STR = "gstate";

static const char * SLX_DETAIL_UNKNOWN_STR = "unknown";
static const char * SLX_DETAIL_VARYING_STR = "varying";
static const char * SLX_DETAIL_UNIFORM_STR = "uniform";

/*
 * Open shader file specified in global 'currentShaderFile', return file pointer
 */
static FILE * OpenCurrentShader()
{
	FILE * shaderInputFile;
	shaderInputFile = NULL;
	if ( currentShaderFilePath != NULL )
	{
		shaderInputFile = fopen( currentShaderFilePath, "r" );
		// Check that the version signature is somewhere in the start 100 bytes, it comes
		// after the shader type, so not at a particular position, but should always be in
		// the first 100 bytes.
		if( NULL != shaderInputFile )
		{
			char sigbuf[100];
			if(fread(sigbuf, sizeof(char), 100, shaderInputFile)<0)
			{
				fclose(shaderInputFile);
				shaderInputFile = NULL;
			}
			else
			{
				fseek(shaderInputFile, 0, SEEK_SET);
				sigbuf[99] = '\0';
				if( strstr(sigbuf, "AQSIS") == NULL )
				{
					fclose( shaderInputFile );
					shaderInputFile = NULL;
				}
			}
		}
	}
	return shaderInputFile;
}


/*
 * Close specified shaderInputFile
 */
static int CloseCurrentShader( FILE * shaderInputFile )
{
	int result;
	result = fclose( shaderInputFile );
	return result;
}


/*
 * Return a pointer to record specified by index from a shader argument array
 */
static SLX_VISSYMDEF * GetShaderArgRecAt( SLX_VISSYMDEF * theShaderArgArray, int argIdx )
{
	return theShaderArgArray + argIdx;
}


/*
 * Return a pointer to record specified by index from a shader argument array
 */
static SLX_VISSYMDEF * GetShaderArgRecByName( SLX_VISSYMDEF * theShaderArgArray,
        int theShaderNArgs, char * name )
{
	//long unsigned int arrayPtr;
	//long unsigned int arrayOffset;
	SLX_VISSYMDEF * recPtr;
	SLX_VISSYMDEF * result;
	bool doLoop;
	int argIdx;

	result = NULL;
	doLoop = true;
	argIdx = 0;

	while ( doLoop == true )
	{
		recPtr = GetShaderArgRecAt( theShaderArgArray, argIdx );
		if ( recPtr != NULL )
		{
			if ( strcmp( name, recPtr->svd_name ) == 0 )
			{
				result = recPtr;	// Success
				doLoop = false;
			}
			else
			{
				argIdx++;
				if ( argIdx >= theShaderNArgs )
					doLoop = false;
			}
		}
		else
		{
			doLoop = false;
		}
	}
	return result;
}


/*
 * Free storage allocated in records of shader argument array
 */
static void FreeArgRecStorage( SLX_VISSYMDEF * theShaderArgArray, int theShaderNArgs )
{
	int argIdx, stringIdx, stringCount;
	for ( argIdx = 0; argIdx < currentShaderNArgs; argIdx++ )
	{
		SLX_VISSYMDEF * theShaderArgRec;
		theShaderArgRec = GetShaderArgRecAt( currentShaderArgsArray, argIdx );

		if ( theShaderArgRec->svd_name != NULL )
		{
			free( theShaderArgRec->svd_name );
			theShaderArgRec->svd_name = NULL;
		}

		if ( theShaderArgRec->svd_spacename != NULL )
		{
			free( theShaderArgRec->svd_spacename );
			theShaderArgRec->svd_spacename = NULL;
		}

		if ( theShaderArgRec->svd_default.stringval != NULL )
		{
			// Delete each string first.
			if ( theShaderArgRec->svd_type == SLX_TYPE_STRING )
			{
				stringCount = 1;
				if ( theShaderArgRec->svd_arraylen != 0 )
					stringCount = theShaderArgRec->svd_arraylen;
				for ( stringIdx = 0; stringIdx < stringCount; stringIdx++ )
					free( theShaderArgRec->svd_default.stringval[ stringIdx ] );
			}

			free( theShaderArgRec->svd_default.stringval );
			theShaderArgRec->svd_default.stringval = NULL;
		}
	}
}


/*
 * Store info in the shader arg record specified by index
*/
static RtInt StoreShaderArgDef( SLX_VISSYMDEF * theArgsArray, int argsArrayIdx,
                                char * varName, SLX_TYPE varType, SLX_STORAGE storage,
								char * spacename, char * defaultVal, int arrayLen )
{
	SLX_VISSYMDEF * theShaderArgRec;
	RtInt result;

	result = RIE_NOERROR;

	theShaderArgRec = GetShaderArgRecAt( theArgsArray, argsArrayIdx );

	theShaderArgRec->svd_name = varName;

	theShaderArgRec->svd_type = varType;

	theShaderArgRec->svd_storage = storage;

	theShaderArgRec->svd_detail = SLX_DETAIL_UNIFORM;

	theShaderArgRec->svd_spacename = spacename;

	theShaderArgRec->svd_arraylen = arrayLen;

	theShaderArgRec->svd_default.stringval = ( char** ) defaultVal;

	return result;
}


/*
 * Allocate storage for array of shader arg records
 */
static RtInt AllocateShaderArgsArray( int varCount, SLX_VISSYMDEF ** newArray )
{
	SLX_VISSYMDEF * arrayStorage;
	RtInt result;

	result = RIE_NOERROR;

	arrayStorage = ( SLX_VISSYMDEF * ) malloc( sizeof( SLX_VISSYMDEF ) * varCount );
	if ( arrayStorage != NULL )
	{
		SLX_VISSYMDEF * theShaderArgRec;
		int i;
		for ( i = 0; i < varCount; i++ )
		{
			theShaderArgRec = GetShaderArgRecAt( arrayStorage, i );
			theShaderArgRec->svd_name = NULL;
			theShaderArgRec->svd_type = SLX_TYPE_UNKNOWN;
			theShaderArgRec->svd_storage = SLX_STOR_UNKNOWN;
			theShaderArgRec->svd_detail = SLX_DETAIL_UNKNOWN;
			theShaderArgRec->svd_spacename = NULL;
			theShaderArgRec->svd_arraylen = 0;
			theShaderArgRec->svd_default.stringval = NULL;
		}
		*newArray = arrayStorage;
	}
	else
	{
		result = RIE_NOMEM;
	}
	return result;
}


/** \brief Extract details about a shader variable.
 *
 * \param shaderVar - variable holding the data to be extracted
 * \param theArgsArray - array into which the shader variable details should be
 *                       stored
 * \param theNArgs - index into theArgsArray.  (Data will be stored in
 *                   theArgsArray[theNArgs] )
 */
static void AddShaderVar( IqShaderData * shaderVar,
                          SLX_VISSYMDEF * theArgsArray, int *theNArgs )
{
	EqVariableType	theType;
	EqVariableClass theClass;
	SLX_TYPE	slxType;
	CqString	varNameCqStr;
	char *	varNameCStr;
	char *	theVarNameStr;
	int nameLength;
	char *	defaultVal = NULL;
	char *	defaultValString = NULL;
	int defaultValLength;
	char * spacename;
	int	arrayLen = 0;
	int	arrayIndex;

	SLX_STORAGE storage = SLX_STOR_UNKNOWN;
	switch(shaderVar->Storage())
	{
		case IqShaderData::Parameter:
			storage = SLX_STOR_PARAMETER;
			break;
		case IqShaderData::OutputParameter:
			storage = SLX_STOR_OUTPUTPARAMETER;
			break;
		default:
			storage = SLX_STOR_UNKNOWN;
			break;
	}

	if ( shaderVar != NULL && storage != SLX_STOR_UNKNOWN )
	{
		theType = shaderVar->Type();
		theClass = shaderVar->Class();

		varNameCqStr = shaderVar->strName();
		varNameCStr = ( char * ) varNameCqStr.c_str();
		nameLength = strlen( varNameCStr );
		theVarNameStr = ( char * ) malloc( nameLength + 1 );
		strcpy( theVarNameStr, varNameCStr );

		spacename = NULL;

		switch ( theType )
		{
				case type_float:
				{
					TqFloat	aTqFloat;
					RtFloat	aRtFloat;
					slxType = SLX_TYPE_SCALAR;
					if ( !shaderVar->isArray())
					{
						shaderVar->GetFloat( aTqFloat );
						aRtFloat = aTqFloat;
						defaultValLength = sizeof( RtFloat );
						defaultVal = ( char * ) malloc( defaultValLength );
						memcpy( defaultVal, &aRtFloat, defaultValLength );
					}
					else
					{
						arrayLen = shaderVar->ArrayLength();
						defaultValLength = sizeof( RtFloat ) * arrayLen;
						defaultVal = ( char * ) malloc( defaultValLength );
						for ( arrayIndex = 0; arrayIndex < arrayLen; arrayIndex++ )
						{
							shaderVar->ArrayEntry( arrayIndex ) ->GetFloat( aTqFloat );
							aRtFloat = aTqFloat;
							memcpy( defaultVal + ( arrayIndex * sizeof( RtFloat ) ), &aRtFloat, sizeof( RtFloat ) );
						}
					}
					spacename = ( char * ) malloc( 1 );
					*spacename = 0x0;	// NULL string
					StoreShaderArgDef( theArgsArray, *theNArgs, theVarNameStr, slxType,
					                   storage, spacename, defaultVal, arrayLen );
					( *theNArgs ) ++;
				}
				break;
				case type_string:
				{
					CqString	aCqString;
					char *	aCString;
					slxType = SLX_TYPE_STRING;
					if ( !shaderVar->isArray() )
					{
						shaderVar->GetString( aCqString );
						aCString = ( char * ) aCqString.c_str();
						defaultValLength = strlen( aCString ) + 1;
						defaultValString = ( char * ) malloc( defaultValLength );
						strcpy( defaultValString, aCString );
						defaultVal = ( char * ) malloc( sizeof( char * ) );
						memcpy( defaultVal, &defaultValString, sizeof( char * ) );
					}
					else
					{
						arrayLen = shaderVar->ArrayLength();
						defaultValLength = sizeof( char * ) * arrayLen;
						defaultVal = ( char * ) malloc( defaultValLength );
						for ( arrayIndex = 0; arrayIndex < arrayLen; arrayIndex++ )
						{
							shaderVar->ArrayEntry( arrayIndex ) ->GetString( aCqString );
							aCString = ( char * ) aCqString.c_str();
							defaultValLength = strlen( aCString ) + 1;
							defaultValString = ( char * ) malloc( defaultValLength );
							strcpy( defaultValString, aCString );
							memcpy( defaultVal + ( arrayIndex * sizeof( char * ) ), &defaultValString, sizeof( char * ) );
						}
					}
					spacename = ( char * ) malloc( 1 );
					*spacename = 0x0;	// NULL string
					StoreShaderArgDef( theArgsArray, *theNArgs, theVarNameStr, slxType,
					                   storage, spacename, defaultVal, arrayLen );
					( *theNArgs ) ++;
				}
				break;
				case type_point:
				{
					CqVector3D	aCqVector3D;
					RtPoint	aRtPoint;
					slxType = SLX_TYPE_POINT;
					if ( !shaderVar->isArray() )
					{
						shaderVar->GetPoint( aCqVector3D );
						aRtPoint[ 0 ] = aCqVector3D[ 0 ];
						aRtPoint[ 1 ] = aCqVector3D[ 1 ];
						aRtPoint[ 2 ] = aCqVector3D[ 2 ];
						defaultValLength = sizeof( RtPoint );
						defaultVal = ( char * ) malloc( defaultValLength );
						memcpy( defaultVal, &aRtPoint, defaultValLength );
					}
					else
					{
						arrayLen = shaderVar->ArrayLength();
						defaultValLength = sizeof( RtPoint ) * arrayLen;
						defaultVal = ( char * ) malloc( defaultValLength );
						for ( arrayIndex = 0; arrayIndex < arrayLen; arrayIndex++ )
						{
							shaderVar->ArrayEntry( arrayIndex ) ->GetPoint( aCqVector3D );
							aRtPoint[ 0 ] = aCqVector3D[ 0 ];
							aRtPoint[ 1 ] = aCqVector3D[ 1 ];
							aRtPoint[ 2 ] = aCqVector3D[ 2 ];
							memcpy( defaultVal + ( arrayIndex * sizeof( RtPoint ) ), &aRtPoint, sizeof( RtPoint ) );
						}
					}

					// shader evaluation space - RI_CURRENT, RI_SHADER, RI_EYE or RI_NDC
					// just go with RI_CURRENT for now
					spacename = ( char * ) malloc( sizeof( "current" ) + 1 );
					strcpy( spacename, "current" );

					StoreShaderArgDef( theArgsArray, *theNArgs, theVarNameStr, slxType,
					                   storage, spacename, defaultVal, arrayLen );
					( *theNArgs ) ++;
				}
				break;
				case type_normal:
				{
					CqVector3D	aCqVector3D;
					RtPoint	aRtPoint;
					slxType = SLX_TYPE_NORMAL;
					if ( !shaderVar->isArray() )
					{
						shaderVar->GetNormal( aCqVector3D );
						aRtPoint[ 0 ] = aCqVector3D[ 0 ];
						aRtPoint[ 1 ] = aCqVector3D[ 1 ];
						aRtPoint[ 2 ] = aCqVector3D[ 2 ];
						defaultValLength = sizeof( RtPoint );
						defaultVal = ( char * ) malloc( defaultValLength );
						memcpy( defaultVal, &aRtPoint, defaultValLength );
					}
					else
					{
						arrayLen = shaderVar->ArrayLength();
						defaultValLength = sizeof( RtPoint ) * arrayLen;
						defaultVal = ( char * ) malloc( defaultValLength );
						for ( arrayIndex = 0; arrayIndex < arrayLen; arrayIndex++ )
						{
							shaderVar->ArrayEntry( arrayIndex ) ->GetNormal( aCqVector3D );
							aRtPoint[ 0 ] = aCqVector3D[ 0 ];
							aRtPoint[ 1 ] = aCqVector3D[ 1 ];
							aRtPoint[ 2 ] = aCqVector3D[ 2 ];
							memcpy( defaultVal + ( arrayIndex * sizeof( RtPoint ) ), &aRtPoint, sizeof( RtPoint ) );
						}
					}

					// shader evaluation space - RI_CURRENT, RI_SHADER, RI_EYE or RI_NDC
					// just go with RI_CURRENT for now
					spacename = ( char * ) malloc( sizeof( "current" ) + 1 );
					strcpy( spacename, "current" );

					StoreShaderArgDef( theArgsArray, *theNArgs, theVarNameStr, slxType,
					                   storage, spacename, defaultVal, arrayLen );
					( *theNArgs ) ++;
				}
				break;
				case type_vector:
				{
					CqVector3D	aCqVector3D;
					RtPoint	aRtPoint;
					slxType = SLX_TYPE_VECTOR;
					if ( !shaderVar->isArray() )
					{
						shaderVar->GetPoint( aCqVector3D );
						aRtPoint[ 0 ] = aCqVector3D[ 0 ];
						aRtPoint[ 1 ] = aCqVector3D[ 1 ];
						aRtPoint[ 2 ] = aCqVector3D[ 2 ];
						defaultValLength = sizeof( RtPoint );
						defaultVal = ( char * ) malloc( defaultValLength );
						memcpy( defaultVal, &aRtPoint, defaultValLength );
					}
					else
					{
						arrayLen = shaderVar->ArrayLength();
						defaultValLength = sizeof( RtPoint ) * arrayLen;
						defaultVal = ( char * ) malloc( defaultValLength );
						for ( arrayIndex = 0; arrayIndex < arrayLen; arrayIndex++ )
						{
							shaderVar->ArrayEntry( arrayIndex ) ->GetPoint( aCqVector3D );
							aRtPoint[ 0 ] = aCqVector3D[ 0 ];
							aRtPoint[ 1 ] = aCqVector3D[ 1 ];
							aRtPoint[ 2 ] = aCqVector3D[ 2 ];
							memcpy( defaultVal + ( arrayIndex * sizeof( RtPoint ) ), &aRtPoint, sizeof( RtPoint ) );
						}
					}

					// shader evaluation space - RI_CURRENT, RI_SHADER, RI_EYE or RI_NDC
					// just go with RI_CURRENT for now
					spacename = ( char * ) malloc( sizeof( "current" ) + 1 );
					strcpy( spacename, "current" );

					StoreShaderArgDef( theArgsArray, *theNArgs, theVarNameStr, slxType,
					                   storage, spacename, defaultVal, arrayLen );
					( *theNArgs ) ++;
				}
				break;
				case type_color:
				{
					CqColor	aCqColor;
					RtColor	aRtColor;
					slxType = SLX_TYPE_COLOR;
					if ( !shaderVar->isArray() )
					{
						shaderVar->GetColor( aCqColor );
						aRtColor[ 0 ] = aCqColor.r();
						aRtColor[ 1 ] = aCqColor.g();
						aRtColor[ 2 ] = aCqColor.b();
						defaultValLength = sizeof( RtColor );
						defaultVal = ( char * ) malloc( defaultValLength );
						memcpy( defaultVal, &aRtColor, defaultValLength );
					}
					else
					{
						arrayLen = shaderVar->ArrayLength();
						defaultValLength = sizeof( RtColor ) * arrayLen;
						defaultVal = ( char * ) malloc( defaultValLength );
						for ( arrayIndex = 0; arrayIndex < arrayLen; arrayIndex++ )
						{
							shaderVar->ArrayEntry( arrayIndex ) ->GetColor( aCqColor );
							aRtColor[ 0 ] = aCqColor[ 0 ];
							aRtColor[ 1 ] = aCqColor[ 1 ];
							aRtColor[ 2 ] = aCqColor[ 2 ];
							memcpy( defaultVal + ( arrayIndex * sizeof( RtColor ) ), &aRtColor, sizeof( RtColor ) );
						}
					}

					// shader evaluation space - RI_RGB, RI_RGBA, RI_RGBZ, RI_RGBAZ, RI_A, RI_Z or RI_AZ
					// just go with RI_RGB for now
					spacename = ( char * ) malloc( sizeof( "rgb" ) + 1 );
					strcpy( spacename, "rgb" );

					StoreShaderArgDef( theArgsArray, *theNArgs, theVarNameStr, slxType,
					                   storage, spacename, defaultVal, arrayLen );
					( *theNArgs ) ++;
				}
				break;
				case type_matrix:
				{
					CqMatrix	aCqMatrix;
					RtMatrix	aRtMatrix;
					slxType = SLX_TYPE_MATRIX;
					if ( !shaderVar->isArray() )
					{
						shaderVar->GetMatrix( aCqMatrix );
						aRtMatrix[ 0 ][ 0 ] = aCqMatrix[ 0 ][ 0 ];
						aRtMatrix[ 0 ][ 1 ] = aCqMatrix[ 0 ][ 1 ];
						aRtMatrix[ 0 ][ 2 ] = aCqMatrix[ 0 ][ 2 ];
						aRtMatrix[ 0 ][ 3 ] = aCqMatrix[ 0 ][ 3 ];
						aRtMatrix[ 1 ][ 0 ] = aCqMatrix[ 1 ][ 0 ];
						aRtMatrix[ 1 ][ 1 ] = aCqMatrix[ 1 ][ 1 ];
						aRtMatrix[ 1 ][ 2 ] = aCqMatrix[ 1 ][ 2 ];
						aRtMatrix[ 1 ][ 3 ] = aCqMatrix[ 1 ][ 3 ];
						aRtMatrix[ 2 ][ 0 ] = aCqMatrix[ 2 ][ 0 ];
						aRtMatrix[ 2 ][ 1 ] = aCqMatrix[ 2 ][ 1 ];
						aRtMatrix[ 2 ][ 2 ] = aCqMatrix[ 2 ][ 2 ];
						aRtMatrix[ 2 ][ 3 ] = aCqMatrix[ 2 ][ 3 ];
						aRtMatrix[ 3 ][ 0 ] = aCqMatrix[ 3 ][ 0 ];
						aRtMatrix[ 3 ][ 1 ] = aCqMatrix[ 3 ][ 1 ];
						aRtMatrix[ 3 ][ 2 ] = aCqMatrix[ 3 ][ 2 ];
						aRtMatrix[ 3 ][ 3 ] = aCqMatrix[ 3 ][ 3 ];
						defaultValLength = sizeof( RtMatrix );
						defaultVal = ( char * ) malloc( defaultValLength );
						memcpy( defaultVal, &aRtMatrix, defaultValLength );
					}
					else
					{
						arrayLen = shaderVar->ArrayLength();
						defaultValLength = sizeof( RtMatrix ) * arrayLen;
						defaultVal = ( char * ) malloc( defaultValLength );
						for ( arrayIndex = 0; arrayIndex < arrayLen; arrayIndex++ )
						{
							shaderVar->ArrayEntry( arrayIndex ) ->GetMatrix( aCqMatrix );
							aRtMatrix[ 0 ][ 0 ] = aCqMatrix[ 0 ][ 0 ];
							aRtMatrix[ 0 ][ 1 ] = aCqMatrix[ 0 ][ 1 ];
							aRtMatrix[ 0 ][ 2 ] = aCqMatrix[ 0 ][ 2 ];
							aRtMatrix[ 0 ][ 3 ] = aCqMatrix[ 0 ][ 3 ];
							aRtMatrix[ 1 ][ 0 ] = aCqMatrix[ 1 ][ 0 ];
							aRtMatrix[ 1 ][ 1 ] = aCqMatrix[ 1 ][ 1 ];
							aRtMatrix[ 1 ][ 2 ] = aCqMatrix[ 1 ][ 2 ];
							aRtMatrix[ 1 ][ 3 ] = aCqMatrix[ 1 ][ 3 ];
							aRtMatrix[ 2 ][ 0 ] = aCqMatrix[ 2 ][ 0 ];
							aRtMatrix[ 2 ][ 1 ] = aCqMatrix[ 2 ][ 1 ];
							aRtMatrix[ 2 ][ 2 ] = aCqMatrix[ 2 ][ 2 ];
							aRtMatrix[ 2 ][ 3 ] = aCqMatrix[ 2 ][ 3 ];
							aRtMatrix[ 3 ][ 0 ] = aCqMatrix[ 3 ][ 0 ];
							aRtMatrix[ 3 ][ 1 ] = aCqMatrix[ 3 ][ 1 ];
							aRtMatrix[ 3 ][ 2 ] = aCqMatrix[ 3 ][ 2 ];
							aRtMatrix[ 3 ][ 3 ] = aCqMatrix[ 3 ][ 3 ];
							memcpy( defaultVal + ( arrayIndex * sizeof( RtMatrix ) ), &aRtMatrix, sizeof( RtMatrix ) );
						}
					}

					// shader evaluation space - RI_CURRENT, RI_SHADER, RI_EYE or RI_NDC
					// just go with RI_CURRENT for now
					spacename = ( char * ) malloc( sizeof( "current" ) + 1 );
					strcpy( spacename, "current" );

					StoreShaderArgDef( theArgsArray, *theNArgs, theVarNameStr, slxType,
					                   storage, spacename, defaultVal, arrayLen );
					( *theNArgs ) ++;
				}
				break;
				default:
				break;
		}
	}
}

/*
 * Read shader info from , set these global variables -
 *    currentShaderType, currentShaderNArgs, currentShaderArgsArray.
 */
static RtInt GetCurrentShaderInfo( char * name, char * filePath )
{
	RtInt result;
	EqShaderType aShaderType;
	SLX_VISSYMDEF * newArray = 0;
	int theNArgs;
	SLX_TYPE theShaderType;

	std::ifstream slxFile(filePath);
	result = RIE_NOERROR;
	theNArgs = 0;

	if ( slxFile )
	{
		try
		{
			boost::shared_ptr<IqShader> pShader = createShaderVM(0, slxFile, DSOPath ? DSOPath : "");
			pShader->SetstrName( filePath );
			pShader->PrepareDefArgs();

			const std::vector<IqShaderData*>& shaderArgs = pShader->GetArguments();
			int varCount = shaderArgs.size();

			AllocateShaderArgsArray( varCount, &newArray );

			theShaderType = SLX_TYPE_UNKNOWN;
			aShaderType = pShader->Type();
			switch ( aShaderType )
			{
					case Type_Surface:
					theShaderType = SLX_TYPE_SURFACE;
					break;
					case Type_Lightsource:
					theShaderType = SLX_TYPE_LIGHT;
					break;
					case Type_Volume:
					theShaderType = SLX_TYPE_VOLUME;
					break;
					case Type_Displacement:
					theShaderType = SLX_TYPE_DISPLACEMENT;
					break;
					case Type_Transformation:
					theShaderType = SLX_TYPE_TRANSFORMATION;
					break;
					case Type_Imager:
					theShaderType = SLX_TYPE_IMAGER;
					break;
			}

			// Iterate through list of shader variables and build array of SLX_VISSYMDEF shader argument records.
			// N.B. Not all shader variables are shader arguments, so NArgs may be less than varCount;
			for ( int i = 0; i < varCount; i++ )
			{
				AddShaderVar( shaderArgs[i], newArray, &theNArgs );
			}

			// Store shader info and arguments array in global storage
			currentShaderArgsArray = newArray;
			currentShaderNArgs = theNArgs;
			currentShaderType = theShaderType;
		}
		catch(XqBadShader& /*e*/)
		{
			// todo: how do we report this error in more detail?
			result = RIE_BADFILE;
		}
	}
	else
	{
		result = RIE_NOFILE;
	}
	return result;
}


/*
 * Return total entry count of shaderSearchPathList
 */
static int GetSearchPathListCount()
{
	int listCharCount;
	int listCharIdx;
	int listElementCount;
	char * currentChar;

	listCharCount = 0;
	listCharIdx = 0;
	listElementCount = 0;
	currentChar = NULL;

	if ( shaderSearchPathList != NULL )
	{
		listCharCount = strlen( shaderSearchPathList );
	}

	if ( listCharCount > 0 )
	{
		listElementCount = 1;
		currentChar = shaderSearchPathList;

		for ( listCharIdx = 0; listCharIdx < listCharCount; listCharIdx++ )
		{
			// Find the next search path in the spec.
			unsigned int len = strcspn( currentChar, ":" );
			// Check if it is really meant as a drive spec.
			if ( len == 1 && isalpha( *currentChar ) )
				len += strcspn( currentChar + 2, ":" ) + 1;

			currentChar += len;
			if ( *currentChar == ':' || *currentChar == 0 ) 	// list elements separated by colons
			{
				listElementCount++;
			}
			currentChar += 1;
			listCharIdx += len;
		}
	}

	return listElementCount;
}


/*
 * Extract indexed entry from search path list, set global currentShaderSearchPath
 */
static int GetSearchPathEntryAtIndex( int pathIdx )
{
	bool doLoop;
	bool entryFound;
	int listEntryIdx;
	char * currentChar;
	char * copyOutChar;
	int listCharCount;
	int listCharIdx;

	doLoop = true;
	entryFound = false;
	listEntryIdx = 0;
	listCharCount = 0;
	listCharIdx = 0;

	if ( currentShaderSearchPath != NULL )
	{
		free( currentShaderSearchPath );
		currentShaderSearchPath = NULL;
	}

	if ( shaderSearchPathList != NULL )
	{
		listCharCount = strlen( shaderSearchPathList );
	}
	currentShaderSearchPath = ( char * ) malloc( listCharCount + 1 );
	currentChar = shaderSearchPathList;
	copyOutChar = currentShaderSearchPath;
	*copyOutChar = 0x0;

	currentChar = shaderSearchPathList;
	for ( listCharIdx = 0; listCharIdx < listCharCount; listCharIdx++ )
	{
		// Find the next search path in the spec.
		unsigned int len = strcspn( currentChar, ":" );
		// Check if it is really meant as a drive spec.
		if ( len == 1 && isalpha( *currentChar ) )
			len += strcspn( currentChar + 2, ":" ) + 1;

		if ( currentChar[len] == ':' || currentChar[len] == 0 ) 	// list elements separated by colons
		{
			listEntryIdx++;
			if( listEntryIdx > pathIdx )
			{
				strncpy( copyOutChar, currentChar, len );
				copyOutChar[len] = '\0';
				return( true );
			}
		}
		currentChar += len + 1;
		listCharIdx += len;
	}

	return entryFound;
}





/*
 * Attempt to open shader file using entries from the search path list and shader name
 * 
 * \pre currentShaderSearchPath must not point to an allocated buffer
 *      (i.e. SLX_EndShader() should have been called before LoadShaderInfo() is called)
 * \pre name must not be NULL
 */
static bool LoadShaderInfo ( char *name )
{
	bool result;
	char * shaderFileName;
	size_t stringLength;
	FILE * shaderInputFile;
	int pathListCount;
	int pathListIdx;
	bool doLoop;

	result = false;
	shaderFileName = NULL;
	stringLength = 0;
	shaderInputFile = NULL;
	pathListCount = 0;
	pathListIdx = 0;
	doLoop = true;

	pathListCount = GetSearchPathListCount();
	//	if ( pathListCount > 0 )
	//	{
	//		if ( GetSearchPathEntryAtIndex( pathListIdx ) == false )
	//			doLoop = false;
	//	}
	//	else
	//	{
	//		doLoop = false;
	//	}
	
	// Build a full file path description
	stringLength = strlen( name ) + sizeof( RI_SHADER_EXTENSION ) + 1;
	shaderFileName = ( char * ) malloc( stringLength );
	if ( shaderFileName != NULL )
	{
		strcpy( shaderFileName, name );
	
		// Check if RI_SHADER_EXTENSION is at the very end of the name
		if ( strlen(name) < strlen(RI_SHADER_EXTENSION) ||
		     strstr( name + strlen( name ) - strlen(RI_SHADER_EXTENSION), RI_SHADER_EXTENSION ) == NULL )
		{
			strcat( shaderFileName, RI_SHADER_EXTENSION );
		}
	
		// --> shaderFileName is now the base shader name + extension
		
		// Check all search paths, always starting with the current directory...
		currentShaderSearchPath = (char*)malloc(strlen("")+1);
		if ( currentShaderSearchPath != NULL )
		{
			strcpy(currentShaderSearchPath, "");
			while ( doLoop == true )
			{
				stringLength = strlen( currentShaderSearchPath ) + strlen( shaderFileName ) + 2;
				if ( currentShaderFilePath != NULL )
				{
					free( currentShaderFilePath );
				}
				currentShaderFilePath = ( char * ) malloc( stringLength );
				if ( currentShaderFilePath != NULL )
				{
					strcpy( currentShaderFilePath, currentShaderSearchPath );
					if( strlen(currentShaderSearchPath) > 0 &&
					        (currentShaderSearchPath[ strlen( currentShaderSearchPath ) - 1 ] != '/') &&
					        (currentShaderSearchPath[ strlen( currentShaderSearchPath ) - 1 ] != '\\') )
						strcat( currentShaderFilePath, "/" );
					strcat( currentShaderFilePath, shaderFileName );
			
					// attempt to open the shader file
					//std::cout << "Trying: " << currentShaderFilePath << std::endl;
					shaderInputFile = OpenCurrentShader();
					if ( shaderInputFile != NULL )
					{
						CloseCurrentShader( shaderInputFile );	// Success
			
						result = GetCurrentShaderInfo(name, currentShaderFilePath) == RIE_NOERROR;
			
						doLoop = false;
					}
			
					if ( result == false )
					{
						if ( GetSearchPathEntryAtIndex( pathListIdx ) == false )
							doLoop = false;
						pathListIdx++;
					}
				}
			}
		}

		free( shaderFileName );
	}
	
	return result;
}




/*
 * Set colon-delimited search path used to locate compiled shaders.
 */
void SLX_SetPath ( char *path )
{
	size_t pathLength;

	pathLength = 0;

	SlxLastError = RIE_NOERROR;
	if ( shaderSearchPathList != NULL )
	{
		free( shaderSearchPathList );
		shaderSearchPathList = NULL;
	}
	if ( path != NULL )
	{
		pathLength = strlen( path );
		shaderSearchPathList = ( char * ) malloc( pathLength + 1 );
		if ( shaderSearchPathList != NULL )
		{
			strcpy( shaderSearchPathList, path );
		}
		else
		{
			SlxLastError = RIE_NOMEM;
		}
	}
}


/*
 * Set search path for DSO libs.
 */
void SLX_SetDSOPath ( char *path )
{
	int pathLength;

	pathLength = 0;

	SlxLastError = RIE_NOERROR;
	if ( DSOPath != NULL )
	{
		free( DSOPath );
		DSOPath = NULL;
	}
	if ( path != NULL )
	{
		pathLength = strlen( path );
		DSOPath = ( char * ) malloc( pathLength + 1 );
		if ( DSOPath != NULL )
		{
			strcpy( DSOPath, path );
		}
		else
		{
			SlxLastError = RIE_NOMEM;
		}
	}
}


/*
 * Return the type of the current shader, enumerated in SLX_TYPE.
 */
char *SLX_GetPath ( void )
{
	SlxLastError = RIE_NOERROR;
	if ( currentShaderSearchPath == NULL )
	{
		SlxLastError = RIE_NOFILE;
	}
	return currentShaderSearchPath;
}


/*
 * Attempt to locate and read the specified compiled shader, 
 * searching entries in currentShaderSearchPath, and set current shader info
 * Return 0 on sucess, -1 on error.
 */
int SLX_SetShader ( char *name )
{
	size_t stringLength;

	stringLength = 0;

	SlxLastError = RIE_NOERROR;

	SLX_EndShader();	// deallocate currentShader and currentShaderFilePath storage

	if ( name == NULL )
	{
		SlxLastError = RIE_NOFILE;
	}
	else
	{
		if ( strcmp( name, "" ) == 0 )
		{
			SlxLastError = RIE_NOFILE;
		}
	}

	if ( SlxLastError == RIE_NOERROR )
	{
		if ( LoadShaderInfo( name ) == false )
			SlxLastError = RIE_NOFILE;
	}

	if ( SlxLastError == RIE_NOERROR )
	{
		stringLength = strlen( name ) + 1;

		// Append RI_SHADER_EXTENSION if not given already
		if ( strlen(name) < strlen(RI_SHADER_EXTENSION) ||
		        strstr( name + strlen( name ) - strlen(RI_SHADER_EXTENSION), RI_SHADER_EXTENSION ) == NULL )
		{
			// Create new string with .slx
			stringLength = strlen( name ) + strlen( RI_SHADER_EXTENSION ) + 1;
			currentShader = ( char * ) malloc( stringLength );
			if ( currentShader != NULL )
			{
				strcpy( currentShader, name );
				strcat( currentShader, RI_SHADER_EXTENSION );
			}
			else
			{
				SlxLastError = RIE_NOMEM;
			}
		}
		else
		{
			currentShader = ( char * ) malloc( stringLength );
			if ( currentShader != NULL )
			{
				strcpy( currentShader, name );
			}
			else
			{
				SlxLastError = RIE_NOMEM;
			}
		}
	}

	return SlxLastError==RIE_NOERROR? 0 : -1;
}


/*
 * Return pointer to a string containing the name of the current shader.
 */
char *SLX_GetName ( void )
{
	SlxLastError = RIE_NOERROR;

	if ( (currentShader == NULL) || (strlen( currentShader ) <= 0) )
	{
		SlxLastError = RIE_NOFILE;
	}

	return currentShader;
}


/*
 * Return type of the current shader, enumerated in SLX_TYPE
 */
SLX_TYPE SLX_GetType ( void )
{
	SLX_TYPE slxType;

	slxType = SLX_TYPE_UNKNOWN;

	SlxLastError = RIE_NOERROR;

	if ( (currentShader != NULL) && (strlen( currentShader ) > 0) )
	{
		slxType = currentShaderType;
	}
	else
	{
		SlxLastError = RIE_NOFILE;
	}

	return slxType;
}


/*
 * Return the number of arguments accepted by the current shader.
 */
int SLX_GetNArgs ( void )
{
	SlxLastError = RIE_NOERROR;
	int result;

	result = 0;

	if ( (currentShader != NULL) && (strlen( currentShader ) > 0) )
	{
		result = currentShaderNArgs;
	}
	else
	{
		SlxLastError = RIE_NOFILE;
	}

	return result;
}


/*
 * Return pointer to shader symbol definition for argument specified by symbol ID
 */
SLX_VISSYMDEF *SLX_GetArgById ( int id )
{
	SLX_VISSYMDEF * result;
	SlxLastError = RIE_NOERROR;
	result = NULL;

	if ( currentShaderArgsArray != NULL )
	{
		if ( id < currentShaderNArgs )
		{
			if ( id >= 0 )
			{
				result = GetShaderArgRecAt( currentShaderArgsArray, id );
			}
		}
	}

	if ( result == NULL )
	{
		SlxLastError = RIE_NOMEM;
	}
	return result;
}


/*
 * Return pointer to shader symbol definition for argument specified by symbol name
 */
SLX_VISSYMDEF *SLX_GetArgByName ( char *name )
{
	SLX_VISSYMDEF * result;
	SlxLastError = RIE_NOERROR;
	result = NULL;

	if ( currentShaderArgsArray != NULL )
	{
		result = GetShaderArgRecByName( currentShaderArgsArray,
		                                currentShaderNArgs, name );
	}

	if ( result == NULL )
	{
		SlxLastError = RIE_NOMEM;
	}
	return result;
}


/*
 *  Return pointer to SLX_VISSYMDEF structure for a single element of an array
 */
SLX_VISSYMDEF *SLX_GetArrayArgElement( SLX_VISSYMDEF *array, int index )
{
	// Not yet implemented
	SlxLastError = RIE_NOERROR;
	return ( NULL );
}


/*
 * Release storage used internally for current shader.
 */
void SLX_EndShader ( void )
{
	SlxLastError = RIE_NOERROR;

	if ( currentShader != NULL )
	{
		free( currentShader );
		currentShader = NULL;
	}
	
	if ( currentShaderFilePath != NULL )
	{
		free( currentShaderFilePath );
		currentShaderFilePath = NULL;
	}

	if ( currentShaderSearchPath != NULL )
	{
		free( currentShaderSearchPath );
		currentShaderSearchPath = NULL;
	}

	FreeArgRecStorage( currentShaderArgsArray, currentShaderNArgs );

	if ( currentShaderArgsArray != NULL )
	{
		free( currentShaderArgsArray );
		currentShaderArgsArray = NULL;
	}

	currentShaderNArgs = 0;

	currentShaderType = SLX_TYPE_UNKNOWN;
}


/*
 * Return ASCII representation of SLX_TYPE
 */
char *SLX_TypetoStr ( SLX_TYPE type )
{
	char * slxTypeStr;

	SlxLastError = RIE_NOERROR;
	slxTypeStr = ( char * ) SLX_TYPE_UNKNOWN_STR;

	switch ( type )
	{
			case SLX_TYPE_UNKNOWN:
			slxTypeStr = ( char * ) SLX_TYPE_UNKNOWN_STR;
			break;
			case SLX_TYPE_POINT:
			slxTypeStr = ( char * ) SLX_TYPE_POINT_STR;
			break;
			case SLX_TYPE_NORMAL:
			slxTypeStr = ( char * ) SLX_TYPE_NORMAL_STR;
			break;
			case SLX_TYPE_VECTOR:
			slxTypeStr = ( char * ) SLX_TYPE_VECTOR_STR;
			break;
			case SLX_TYPE_COLOR:
			slxTypeStr = ( char * ) SLX_TYPE_COLOR_STR;
			break;
			case SLX_TYPE_SCALAR:
			slxTypeStr = ( char * ) SLX_TYPE_SCALAR_STR;
			break;
			case SLX_TYPE_STRING:
			slxTypeStr = ( char * ) SLX_TYPE_STRING_STR;
			break;
			case SLX_TYPE_SURFACE:
			slxTypeStr = ( char * ) SLX_TYPE_SURFACE_STR;
			break;
			case SLX_TYPE_LIGHT:
			slxTypeStr = ( char * ) SLX_TYPE_LIGHT_STR;
			break;
			case SLX_TYPE_DISPLACEMENT:
			slxTypeStr = ( char * ) SLX_TYPE_DISPLACEMENT_STR;
			break;
			case SLX_TYPE_VOLUME:
			slxTypeStr = ( char * ) SLX_TYPE_VOLUME_STR;
			break;
			case SLX_TYPE_TRANSFORMATION:
			slxTypeStr = ( char * ) SLX_TYPE_TRANSFORMATION_STR;
			break;
			case SLX_TYPE_IMAGER:
			slxTypeStr = ( char * ) SLX_TYPE_IMAGER_STR;
			break;
			case SLX_TYPE_MATRIX:
			slxTypeStr = ( char * ) SLX_TYPE_MATRIX_STR;
			break;

	}
	return slxTypeStr;
}


/*
 * Return ASCII representation of SLX_STORAGE
 */
char *SLX_StortoStr ( SLX_STORAGE storage )
{
	char * slxStorageStr;

	SlxLastError = RIE_NOERROR;
	slxStorageStr = ( char * ) SLX_STOR_UNKNOWN_STR;

	switch ( storage )
	{
			case SLX_STOR_UNKNOWN:
			slxStorageStr = ( char * ) SLX_STOR_UNKNOWN_STR;
			break;
			case SLX_STOR_CONSTANT:
			slxStorageStr = ( char * ) SLX_STOR_CONSTANT_STR;
			break;
			case SLX_STOR_VARIABLE:
			slxStorageStr = ( char * ) SLX_STOR_VARIABLE_STR;
			break;
			case SLX_STOR_TEMPORARY:
			slxStorageStr = ( char * ) SLX_STOR_TEMPORARY_STR;
			break;
			case SLX_STOR_PARAMETER:
			slxStorageStr = ( char * ) SLX_STOR_PARAMETER_STR;
			break;
			case SLX_STOR_OUTPUTPARAMETER:
			slxStorageStr = ( char * ) SLX_STOR_OUTPUTPARAMETER_STR;
			break;
			case SLX_STOR_GSTATE:
			slxStorageStr = ( char * ) SLX_STOR_GSTATE_STR;
			break;
	}
	return slxStorageStr;
}


/*
 * Return ASCII representation of SLX_DETAIL
 */
char *SLX_DetailtoStr ( SLX_DETAIL detail )
{
	char * slxDetailStr;

	SlxLastError = RIE_NOERROR;
	slxDetailStr = ( char * ) SLX_DETAIL_UNKNOWN_STR;

	switch ( detail )
	{
			case SLX_DETAIL_UNKNOWN:
			slxDetailStr = ( char * ) SLX_DETAIL_UNKNOWN_STR;
			break;
			case SLX_DETAIL_VARYING:
			slxDetailStr = ( char * ) SLX_DETAIL_VARYING_STR;
			break;
			case SLX_DETAIL_UNIFORM:
			slxDetailStr = ( char * ) SLX_DETAIL_UNIFORM_STR;
			break;
	}
	return slxDetailStr;
}

