// Aqsis
// Copyright © 2001, Paul C. Gregory
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
		\brief Implements CqPoints primitives using regular polygon (first try).
		\author M. Joron (joron@sympatico.ca)
*/ 


//? Is .h included already?
#ifndef POINTS_H_INCLUDED
#define POINTS_H_INCLUDED

#include	"aqsis.h"
#include	"matrix.h"
#include	"surface.h"
#include	"vector4d.h"

#include	"ri.h"

#include        "polygon.h"

START_NAMESPACE( Aqsis )

//----------------------------------------------------------------------
/** \class CqPoints
 * Class encapsulating the functionality of Points geometry.
 */

class CqPoints : public CqSurface
{
	public:
		
		CqPoints( TqInt n, TqFloat *origins,TqFloat *sizes, TqFloat constantwidth);
		CqPoints( const CqPoints& From )
		{
			*this = From;
		}
		virtual	~CqPoints()
		{}

		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx );


		virtual	TqUint	cUniform() const
		{
			return ( 1 );
		}
		virtual	TqUint	cVarying() const
		{
			return ( 4 );
		}
		virtual	TqUint	cVertex() const
		{
			return ( 16 );
		}
		virtual	TqUint	cFaceVarying() const
		{
			/// \todo Must work out what this value should be.
			return ( 1 );
		}
		// Overrides from CqSurface
		virtual	CqMicroPolyGridBase* Dice();
		virtual TqBool	Diceable();

		virtual	CqBound	Bound() const;
		virtual	TqInt	Split( std::vector<CqBasicSurface*>& aSplits );

		CqPoints&	operator=( const CqPoints& From );
		
		virtual	TqUint	n() const
		{
			return ( m_n );
		}
		
		
		std::vector <CqSurfacePolygon*> m_pPolygons;
		
		
	private:
	
		TqInt				  m_n;
		
	protected:
		CqMatrix	m_matTx;		///< Transformation matrix from object to camera.
		CqMatrix	m_matITTx;		///< Inverse transpose transformation matrix, for transforming normals.
}
;


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !POINTS_H_INCLUDED
