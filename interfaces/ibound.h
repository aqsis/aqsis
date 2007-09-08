// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Declares an abstact interface to a geometric bound class.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef IBOUND_H_INCLUDED
#define IBOUND_H_INCLUDED 1

#include	"aqsis.h"

#include	"matrix.h"

START_NAMESPACE( Aqsis )

class CqPlane;

//----------------------------------------------------------------------
/** \struct IqBound
 * Abstract interface onto a class that implements geometric bounds.
 */

struct IqBound
{
	virtual const	CqVector3D&	vecMin() const = 0;
	virtual CqVector3D&	vecMin() = 0;
	virtual const	CqVector3D&	vecMax() const = 0;
	virtual CqVector3D&	vecMax() = 0;
	virtual CqVector3D vecCross() const = 0;
	virtual TqFloat	Volume() const = 0;
	virtual TqFloat Volume2() const = 0;

	virtual void		Transform( const CqMatrix&	matTransform ) = 0;
	virtual void		Encapsulate( const IqBound* const bound ) = 0;
	virtual void		Encapsulate( const CqVector3D& v ) = 0;
	virtual void		Encapsulate( const CqVector2D& v ) = 0;

	virtual bool	Contains2D( const IqBound* const b ) const = 0;
	virtual bool	Contains3D( const CqVector3D& v ) const = 0;
	virtual bool	Contains2D( const CqVector2D& v ) const = 0;
	virtual bool	Intersects( const CqVector2D& min, const CqVector2D& max ) const = 0;

	enum EqPlaneSide
	{
		Side_Outside = -1,
		Side_Both = 0,
		Side_Inside = 1,
	};

	virtual TqInt whichSideOf(const CqPlane* const plane) const = 0;
};
//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif // BOUND_H_INCLUDED
