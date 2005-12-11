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
		\brief Implements libslxargs, analogous to Pixar's sloarg library.
		\author Douglas Ward (dsward@vidi.com)
*/

#ifndef SLX_H_INCLUDED
#define SLX_H_INCLUDED 1

#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum {
	    SLX_TYPE_UNKNOWN,
	    SLX_TYPE_POINT,
	    SLX_TYPE_COLOR,
	    SLX_TYPE_SCALAR,
	    SLX_TYPE_STRING,
	    SLX_TYPE_SURFACE,
	    SLX_TYPE_LIGHT,
	    SLX_TYPE_DISPLACEMENT,
	    SLX_TYPE_VOLUME,
	    SLX_TYPE_TRANSFORMATION,
	    SLX_TYPE_IMAGER,
	    SLX_TYPE_VECTOR,
	    SLX_TYPE_NORMAL,
	    SLX_TYPE_MATRIX
	} SLX_TYPE;

	typedef enum {
	    SLX_STOR_UNKNOWN,
	    SLX_STOR_CONSTANT,
	    SLX_STOR_VARIABLE,
	    SLX_STOR_TEMPORARY,
	    SLX_STOR_PARAMETER,
	    SLX_STOR_GSTATE
	} SLX_STORAGE;

	typedef enum {
	    SLX_DETAIL_UNKNOWN,
	    SLX_DETAIL_VARYING,
	    SLX_DETAIL_UNIFORM
	} SLX_DETAIL;

	typedef struct
	{
		float	xval;
		float	yval;
		float	zval;
	}
	SLX_POINT;

	typedef struct
	{
		float	val[4][4];
	}
	SLX_MATRIX;

	typedef float SLX_SCALAR;

	typedef struct SLXvissymdef
	{
		char *svd_name;
		SLX_TYPE svd_type;
		SLX_STORAGE svd_storage;
		SLX_DETAIL svd_detail;
		char *	svd_spacename;
		int	svd_arraylen;
		union {
			SLX_POINT	*pointval;
			SLX_SCALAR	*scalarval;
			SLX_MATRIX	*matrixval;
			char	**stringval;
		} svd_default;
	}
	SLX_VISSYMDEF;

#define NULL_SLXVISSYMDEF ((SLX_VISSYMDEF *)0)

	extern void SLX_SetPath ( char * path );
	extern char *SLX_GetPath ( void );
	extern int SLX_SetShader ( char * name );
	extern char *SLX_GetName ( void );
	extern SLX_TYPE SLX_GetType ( void );
	extern int SLX_GetNArgs ( void );
	extern SLX_VISSYMDEF *SLX_GetArgById ( int id );
	extern SLX_VISSYMDEF *SLX_GetArgByName ( char * name );
	extern SLX_VISSYMDEF *SLX_GetArrayArgElement( SLX_VISSYMDEF * array, int index );
	extern void SLX_EndShader ( void );
	extern char *SLX_TypetoStr ( SLX_TYPE type );
	extern char *SLX_StortoStr ( SLX_STORAGE storage );
	extern char *SLX_DetailtoStr ( SLX_DETAIL detail );
	extern void SLX_SetDSOPath ( char * path );

#ifdef __cplusplus
}
#endif

#endif /* SLX_H_INCLUDED */
