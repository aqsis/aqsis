// Aqsis
// Copyright (C) 2001, Paul C. Gregory
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
		\brief Implements RiGeometry "teapot" option.
		\author T. Burge (tburge@affine.org)
*/
/*    References:
 *          [CROW87]  Crow, F. C. The Origins of the Teapot. 
 *                    IEEE Computer Graphics and Applications, 
 *                    pp. 8-19, Vol 7 No 1, 1987.
 *          [PIXA89]  Pixar, The RenderMan Interface, Version 3.1, 
 *                    Richmond, CA, September 1989.  
 *
 */

//? Is .h included already?
#ifndef TEAPOT_H_INCLUDED
#define TEAPOT_H_INCLUDED

#include	<aqsis/aqsis.h>
#include	<aqsis/math/matrix.h>
#include	"surface.h"

namespace Aqsis {

//----------------------------------------------------------------------
/** \class CqTeapot
 * Class encapsulating the functionality of teapot geometry.
 */

class CqTeapot : public CqSurface
{
	public:
		CqTeapot( bool addCrowBase = false );
		virtual	~CqTeapot()
		{}

		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime = 0 );

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual bool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( false );
		}

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
			return ( cVarying() );
		}

		boost::shared_ptr<CqSurface> pPatchMeshBicubic[ 7 ];
		TqInt cNbrPatchMeshBicubic;

		// Overrides from CqSurface
		virtual	void	Bound(CqBound* bound) const;
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );

		virtual CqSurface* Clone() const;

	private:
		bool	m_CrowBase;			///< Utah teapot was missing a bottom.  F. Crow added one.

	protected:
		CqMatrix	m_matTx;		///< Transformation matrix from object to camera.
		CqMatrix	m_matITTx;		///< Inverse transpose transformation matrix, for transforming normals.
}
;


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !TEAPOT_H_INCLUDED
