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
 * \brief Definitions of types used by the RenderMan Interface
 *
 * ===================================================================
 * C-compatible header. C++ constructs must be preprocessor-protected.
 * ===================================================================
 */

#ifndef RI_TYPES_H_INCLUDED
#define RI_TYPES_H_INCLUDED

#ifdef __cplusplus
extern	"C"
{
#endif

typedef	short	RtBoolean;
typedef	int		RtInt;
typedef	float	RtFloat;

typedef	char*	RtToken;

typedef	RtFloat	RtColor[ 3 ];
typedef	RtFloat	RtPoint[ 3 ];
typedef	RtFloat	RtMatrix[ 4 ][ 4 ];
typedef	RtFloat	RtBasis[ 4 ][ 4 ];
typedef	RtFloat	RtBound[ 6 ];
typedef	char*	RtString;

typedef	void*	RtPointer;
typedef	void	RtVoid;

typedef	RtFloat	( *RtFilterFunc ) ( RtFloat, RtFloat, RtFloat, RtFloat );
typedef	RtFloat	( *RtFloatFunc ) ();
typedef	RtVoid	( *RtFunc ) ();
typedef	RtVoid	( *RtErrorFunc ) ( RtInt code, RtInt severity, RtString message );
typedef	RtErrorFunc	RtErrorHandler;

typedef	RtVoid	( *RtProcSubdivFunc ) ( RtPointer, RtFloat );
typedef	RtVoid	( *RtProcFreeFunc ) ( RtPointer );
typedef	RtVoid	( *RtArchiveCallback ) ( RtToken, char *, ... );

typedef	RtPointer	RtObjectHandle;
typedef	RtPointer	RtLightHandle;
typedef	RtPointer	RtContextHandle;


/* Aqsis-specific typedefs */
typedef	RtVoid	( *RtProgressFunc ) ( RtFloat PercentComplete, RtInt FrameNo );

#ifdef	__cplusplus
}
#endif

#endif // RI_TYPES_H_INCLUDED
