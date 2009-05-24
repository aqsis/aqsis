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
		\brief Declares a class for handling general polygons with loops.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#ifndef	___genpoly_Loaded___
#define	___genpoly_Loaded___



#include	<vector>

#include	<aqsis/aqsis.h>
#include	<aqsis/math/vector2d.h>
#include	<aqsis/math/vector3d.h>
#include	"surface.h"

#include	<aqsis/ri/ri.h>

namespace Aqsis {

//----------------------------------------------------------------------
/** \class CqPolygonGeneral2D
 * A Genral polygon in 2 dimensions, used for the glyph conversions.
 * The representation of this poygon is designed to allow non convex 
 * polygon support, with holes, to be triangulated on request.
 */

class CqPolygonGeneral2D
{
	public:
		CqPolygonGeneral2D() :
				m_Orientation( Orientation_Unknown ),
				m_Reverse( false )
		{
			STATS_INC( GPR_poly );
		}
		CqPolygonGeneral2D( const CqPolygonGeneral2D& From );
		~CqPolygonGeneral2D()
		{}

		std::vector<TqInt>&	aiVertices()
		{
			return ( m_aiVertices );
		}
		TqInt	cVertices() const
		{
			return ( m_aiVertices.size() );
		}
		TqInt	Orientation() const
		{
			return ( m_Orientation );
		}
		TqInt	Axis() const
		{
			return ( m_Axis );
		}
		void	SetAxis( TqInt axis )
		{
			m_Axis = axis;
		}
		/** \brief Compute and set the projection axis for the polygon.
		 *
		 * The projection axis is chosen such that projecting that axis out
		 * leaves the polyogon nondegenerate.
		 */
		void CalcAxis();
		void	SetpVertices( const boost::shared_ptr<CqSurface>& pVertices )
		{
			m_pVertices = pVertices;
		}

		void	SwapDirection();
		TqInt	CalcOrientation();
		TqInt	CalcDeterminant( TqInt i1, TqInt i2, TqInt i3 ) const;
		bool	NoneInside( TqInt P1, TqInt P2, TqInt P3, std::vector<TqInt>& iList ) const;
		void	EliminateDuplicatePoints();

		bool	Contains( CqPolygonGeneral2D& polyCheck );
		void	Combine( CqPolygonGeneral2D& polyFrom );
		void	Triangulate( std::vector<TqInt>& aiList ) const;

		CqVector2D	operator[] ( TqInt index ) const
		{
			switch ( m_Axis )
			{
					case Axis_XY:
					return ( CqVector2D( m_pVertices->P()->pValue( m_aiVertices[ index ] )[0].x(),
					                     m_pVertices->P()->pValue( m_aiVertices[ index ] )[0].y() ) );

					case Axis_XZ:
					return ( CqVector2D( m_pVertices->P()->pValue( m_aiVertices[ index ] )[0].x(),
					                     m_pVertices->P()->pValue( m_aiVertices[ index ] )[0].z() ) );

					case Axis_YZ:
					return ( CqVector2D( m_pVertices->P()->pValue( m_aiVertices[ index ] )[0].y(),
					                     m_pVertices->P()->pValue( m_aiVertices[ index ] )[0].z() ) );
			}
			return ( CqVector2D( 0, 0 ) );
		}
		CqPolygonGeneral2D& operator=( const CqPolygonGeneral2D& From );

		enum	EssOrientation
		{
		    Orientation_Unknown,

		    Orientation_Clockwise,
		    Orientation_AntiClockwise,
	};
		enum	EssAxis
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
		boost::shared_ptr<CqSurface>	m_pVertices;
		bool	m_Reverse;
};


//-----------------------------------------------------------------------

} // namespace Aqsis


#endif	//	___genpoly_Loaded___
