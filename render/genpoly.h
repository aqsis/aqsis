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
		\brief Declares a class for handling general polygons with loops.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#ifndef	___genpoly_Loaded___
#define	___genpoly_Loaded___



#include	<vector>

#include	"aqsis.h"
#include	"vector2d.h"
#include	"vector3d.h"
#include	"surface.h"

#include	"ri.h"

#define		_qShareName	CORE
#include	"share.h"


START_NAMESPACE( Aqsis )

//----------------------------------------------------------------------
/** \class CqPolygonGeneral2D
 * A Genral polygon in 2 dimensions, used for the glyph conversions.
 * The representation of this poygon is designed to allow non convex 
 * polygon support, with holes, to be triangulated on request.
 */

class _qShareC	CqPolygonGeneral2D
{
	public:
		_qShareM	CqPolygonGeneral2D() :
				m_Orientation( Orientation_Unknown )
		{}
		_qShareM	CqPolygonGeneral2D( const CqPolygonGeneral2D& From );
		_qShareM	~CqPolygonGeneral2D()
		{
			if ( m_pVertices )
				m_pVertices->Release();
		}

		_qShareM	std::vector<TqInt>&	aiVertices()
		{
			return ( m_aiVertices );
		}
		_qShareM	TqInt	cVertices() const
		{
			return ( m_aiVertices.size() );
		}
		_qShareM	TqInt	Orientation() const
		{
			return ( m_Orientation );
		}
		_qShareM	TqInt	Axis() const
		{
			return ( m_Axis );
		}
		_qShareM	void	SetAxis( TqInt axis )
		{
			m_Axis = axis;
		}
		_qShareM	void	SetpVertices( CqSurface* pVertices )
		{
			m_pVertices = pVertices;
			pVertices->AddRef();
		}

		_qShareM	void	SwapDirection();
		_qShareM	TqInt	CalcOrientation();
		_qShareM	TqInt	CalcDeterminant( TqInt i1, TqInt i2, TqInt i3 ) const;
		_qShareM	TqBool	NoneInside( TqInt P1, TqInt P2, TqInt P3, std::vector<TqInt>& iList ) const;
		_qShareM	void	EliminateDuplicatePoints();

		_qShareM	TqBool	Contains( CqPolygonGeneral2D& polyCheck );
		_qShareM	void	Combine( CqPolygonGeneral2D& polyFrom );
		_qShareM	void	Triangulate( std::vector<TqInt>& aiList ) const;

		_qShareM	CqVector2D	operator[] ( TqInt index ) const
		{
			switch ( m_Axis )
			{
					case Axis_XY:
					return ( CqVector2D( ( *m_pVertices->P() ) [ m_aiVertices[ index ] ].x(),
					                     ( *m_pVertices->P() ) [ m_aiVertices[ index ] ].y() ) );

					case Axis_XZ:
					return ( CqVector2D( ( *m_pVertices->P() ) [ m_aiVertices[ index ] ].x(),
					                     ( *m_pVertices->P() ) [ m_aiVertices[ index ] ].z() ) );

					case Axis_YZ:
					return ( CqVector2D( ( *m_pVertices->P() ) [ m_aiVertices[ index ] ].y(),
					                     ( *m_pVertices->P() ) [ m_aiVertices[ index ] ].z() ) );
			}
			return ( CqVector2D( 0, 0 ) );
		}
		_qShareM	CqPolygonGeneral2D& operator=( const CqPolygonGeneral2D& From );

		_qShareM	enum	EssOrientation
		{
		    Orientation_Unknown,

		    Orientation_Clockwise,
		    Orientation_AntiClockwise,
	};
		_qShareM	enum	EssAxis
		{
		    Axis_Unknown,

		    Axis_XY,
		    Axis_XZ,
		    Axis_YZ,
	};
	private:
		std::vector<TqInt>	m_aiVertices;
		TqInt	m_Orientation;
		TqInt	m_Axis;
		CqSurface*	m_pVertices;
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )


#endif	//	___genpoly_Loaded___
