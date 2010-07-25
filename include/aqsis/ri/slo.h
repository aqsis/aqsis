/* Aqsis
 / Copyright (C) 1997 - 2007, Paul C. Gregory
 /
 / Contact: pgregory@aqsis.org
 /
 / This library is free software; you can redistribute it and/or
 / modify it under the terms of the GNU General Public
 / License as published by the Free Software Foundation; either
 / version 2 of the License, or (at your option) any later version.
 /
 / This library is distributed in the hope that it will be useful,
 / but WITHOUT ANY WARRANTY; without even the implied warranty of
 / MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 / General Public License for more details.
 /
 / You should have received a copy of the GNU General Public
 / License along with this library; if not, write to the Free Software
 / Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	 02111-1307	 USA
*/

/** \file
 *
 * \brief Compatibility interface to Pixar's sloarg shader argument library.
 * \author Matthias Baas
 * \author Malcolm Humphreys
 *
 * ===================================================================
 * C-compatible header. C++ constructs must be preprocessor-protected.
 * ===================================================================
 */

#ifndef SLO_H_INCLUDED
#define SLO_H_INCLUDED

#include <aqsis/ri/slx.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
	SLO_TYPE_UNKNOWN,
	SLO_TYPE_POINT,
	SLO_TYPE_COLOR,
	SLO_TYPE_SCALAR,
	SLO_TYPE_STRING,
	SLO_TYPE_SURFACE,
	SLO_TYPE_LIGHT,
	SLO_TYPE_DISPLACEMENT,
	SLO_TYPE_VOLUME,
	SLO_TYPE_TRANSFORMATION,
	SLO_TYPE_IMAGER,
	SLO_TYPE_VECTOR,
	SLO_TYPE_NORMAL,
	SLO_TYPE_MATRIX,
	SLO_TYPE_SHADER // not-supported
} SLO_TYPE;

typedef enum
{
	SLO_STOR_UNKNOWN,
	SLO_STOR_CONSTANT,
	SLO_STOR_VARIABLE,
	SLO_STOR_TEMPORARY,
	SLO_STOR_PARAMETER,
	SLO_STOR_OUTPUTPARAMETER,
	SLO_STOR_GSTATE
} SLO_STORAGE;

typedef enum
{
	SLO_DETAIL_UNKNOWN,
	SLO_DETAIL_VARYING,
	SLO_DETAIL_UNIFORM
} SLO_DETAIL;

typedef struct
{
	float xval;
	float yval;
	float zval;
} SLO_POINT;

typedef float SLO_SCALAR;

typedef float* SLO_MATRIX;

typedef struct SLOvissymdef
{
	const char *svd_name;
	SLO_TYPE svd_type;
	SLO_STORAGE svd_storage;
	SLO_DETAIL svd_detail;
	const char *svd_spacename;
	
	union {
		const SLO_POINT *pointval;
		const SLO_SCALAR *scalarval;
		const char *stringval;
		const float *matrixval;
	} svd_default;
	
	unsigned svd_valisvalid:1;
	int svd_arraylen;
} SLO_VISSYMDEF;

AQSIS_SLXARGS_SHARE void Slo_SetPath ( char* path );
AQSIS_SLXARGS_SHARE int Slo_SetShader ( char* name );
AQSIS_SLXARGS_SHARE const char* Slo_GetName ( void );
AQSIS_SLXARGS_SHARE SLO_TYPE Slo_GetType ( void );
AQSIS_SLXARGS_SHARE int Slo_HasMethod(const char *i_name);
AQSIS_SLXARGS_SHARE const char* const* Slo_GetMethodNames();
AQSIS_SLXARGS_SHARE int Slo_GetNArgs (void);

AQSIS_SLXARGS_SHARE SLO_VISSYMDEF* Slo_GetArgById ( int id );
AQSIS_SLXARGS_SHARE SLO_VISSYMDEF* Slo_GetArgByName ( char* name );
AQSIS_SLXARGS_SHARE SLO_VISSYMDEF* Slo_GetArrayArgElement( SLO_VISSYMDEF* array, int index );

AQSIS_SLXARGS_SHARE void Slo_EndShader ( void );
AQSIS_SLXARGS_SHARE char* Slo_TypetoStr ( SLO_TYPE type );
AQSIS_SLXARGS_SHARE char* Slo_StortoStr ( SLO_STORAGE storage );
AQSIS_SLXARGS_SHARE char* Slo_DetailtoStr ( SLO_DETAIL detail );

#ifdef __cplusplus
}
#endif

#endif /* SLO_H_INCLUDED */
