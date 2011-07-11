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
		\brief Declares the classes and support structures for handling RenderMan patch primitives.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef PATCH_H_INCLUDED
#define PATCH_H_INCLUDED 1

#include	<aqsis/aqsis.h>

#include	<aqsis/math/matrix.h>
#include	"surface.h"
#include	<aqsis/math/vector4d.h>

namespace Aqsis {

class CqMicroPolyGrid;
class CqMicroPolyGridBase;

//----------------------------------------------------------------------
/** \class CqSurfacePatchBicubic
 * Bicubic spline patch
 */

class CqSurfacePatchBicubic : public CqSurface
{
	public:
		CqSurfacePatchBicubic();
		virtual	~CqSurfacePatchBicubic();

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqSurfacePatchBicubic");
		}
#endif

		/** Get a reference to the indexed control point.
		 * \param iRow Integer row index.
		 * \param iCol Integer column index.
		 * \return CqVector4D reference.
		 */
		const	CqVector4D& CP( TqInt iRow, TqInt iCol ) const
		{
			return ( P()->pValue( ( iRow * 4 ) + iCol )[0] );
		}
		/** Get a reference to the indexed control point.
		 * \param iRow Integer row index.
		 * \param iCol Integer column index.
		 * \return CqVector4D reference.
		 */
		CqVector4D& CP( TqInt iRow, TqInt iCol )
		{
			return ( P()->pValue( ( iRow * 4 ) + iCol )[0] );
		}

		virtual	void	Bound(CqBound* bound) const;
		virtual bool	Diceable(const CqMatrix& matCtoR);

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
		virtual	TqUint cFaceVarying() const
		{
			return ( cVarying() );
		}

		virtual void NaturalDice( CqParameter* pParameter, TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pData );
		virtual	TqInt PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u );
		virtual void NaturalSubdivide( CqParameter* pParam, CqParameter* pParam1, CqParameter* pParam2, bool u );

		void	ConvertToBezierBasis( CqMatrix& matuBasis, CqMatrix& matvBasis );

		virtual CqSurface* Clone() const;

	protected:
};


//----------------------------------------------------------------------
/** \class CqSurfacePatchBilinear
 * Bilinear spline patch
 */

class CqSurfacePatchBilinear : public CqSurface
{
	public:
		CqSurfacePatchBilinear();
		virtual	~CqSurfacePatchBilinear();

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqSurfacePatchBilinear");
		}
#endif

		void	SetfHasPhantomFourthVertex(bool fHasPhantomFourthVertex)
		{
			m_fHasPhantomFourthVertex = fHasPhantomFourthVertex;
		}

		bool	fHasPhantomFourthVertex() const
		{
			return(m_fHasPhantomFourthVertex);
		}

		void	GenNormals();
		virtual	void	Bound(CqBound* bound) const;
		virtual bool	Diceable(const CqMatrix& matCtoR);

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
			return ( 4 );
		}
		virtual	TqUint	cFaceVarying() const
		{
			return ( cVarying() );
		}
		virtual CqSurface* Clone() const;

		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		virtual	TqInt	PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u );
		virtual void	PostDice(CqMicroPolyGrid * pGrid);

	protected:
		bool	m_fHasPhantomFourthVertex;
		TqInt	m_iInternalu;
		TqInt	m_iInternalv;
};


//----------------------------------------------------------------------
/** \class CqSurfacePatchMeshBicubic
 * Bicubic spline patch mesh
 */

class CqSurfacePatchMeshBicubic : public CqSurface
{
	public:
		CqSurfacePatchMeshBicubic() :
				CqSurface(),
				m_nu( 0 ),
				m_nv( 0 ),
				m_uPeriodic( false ),
				m_vPeriodic( false )
		{}
		CqSurfacePatchMeshBicubic( TqInt nu, TqInt nv, bool uPeriodic = false, bool vPeriodic = false ) :
				CqSurface(),
				m_nu( nu ),
				m_nv( nv ),
				m_uPeriodic( uPeriodic ),
				m_vPeriodic( vPeriodic )
		{
			TqInt uStep = pAttributes() ->GetIntegerAttribute( "System", "BasisStep" ) [ 0 ];
			TqInt vStep = pAttributes() ->GetIntegerAttribute( "System", "BasisStep" ) [ 1 ];
			m_uPatches = ( uPeriodic ) ? nu / uStep : ( ( nu - 4 ) / uStep ) + 1;
			m_vPatches = ( vPeriodic ) ? nv / vStep : ( ( nv - 4 ) / vStep ) + 1;
		}
		virtual	~CqSurfacePatchMeshBicubic();

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqSurfacePatchMeshBicubic");
		}
#endif

		virtual	void	SetDefaultPrimitiveVariables( bool bUseDef_st = true )
		{}

		virtual	void	Bound(CqBound* bound) const;
		virtual	CqMicroPolyGridBase* Dice()
		{
			return ( 0 );
		}
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		virtual bool	Diceable(const CqMatrix& /*matCtoR*/)
		{
			return ( false );
		}

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual bool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( false );
		}

		virtual	TqUint	cUniform() const
		{
			return ( m_uPatches * m_vPatches );
		}
		virtual	TqUint	cVarying() const
		{
			return ( ( ( m_uPeriodic ) ? m_uPatches : m_uPatches + 1 ) * ( ( m_vPeriodic ) ? m_vPatches : m_vPatches + 1 ) );
		}
		virtual	TqUint	cVertex() const
		{
			return ( m_nu * m_nv );
		}
		virtual	TqUint	cFaceVarying() const
		{
			return ( cVarying() );
		}
		virtual CqSurface* Clone() const;

		virtual CqVector3D	SurfaceParametersAtVertex( TqInt index )
		{
			CqVector3D	vec( 0, 0, 0 );
			TqFloat u = static_cast<TqFloat>( index % m_nu );
			u /= ( m_nu - 1 );
			TqFloat v = static_cast<TqFloat>( index / m_nu );
			v /= ( m_nv - 1 );
			vec.x( u );
			vec.y( v );
			return ( vec );
		}

		virtual void Transform(const CqMatrix& matTx, const CqMatrix& matITTx,
							   const CqMatrix& matRTx, TqInt iTime = 0 );

		virtual void ConvertToBezierBasis();

	protected:

		TqInt	m_uPatches,   	///< Number of patches in u.
		m_vPatches;				///< Number of patches in v.
		TqInt	m_nu,   		///< Number of control points in u.
		m_nv;					///< Number of control points in v.
		bool	m_uPeriodic,   	///< Is patches mesh periodic in u?
		m_vPeriodic;			///< Is patches mesh periodic in v?

	private:

		void SplitInternally();
		/** Child bicubic patches. */
		std::vector<boost::shared_ptr<CqSurfacePatchBicubic> > m_patches;
}
;


//----------------------------------------------------------------------
/** \class CqSurfacePatchMeshlinear
 * Bilinear spline patch mesh
 */

class CqSurfacePatchMeshBilinear : public CqSurface
{
	public:
		CqSurfacePatchMeshBilinear() :
				CqSurface(),
				m_nu( 0 ),
				m_nv( 0 ),
				m_uPeriodic( false ),
				m_vPeriodic( false )
		{}
		CqSurfacePatchMeshBilinear( TqInt nu, TqInt nv, bool uPeriodic = false, bool vPeriodic = false ) :
				CqSurface(),
				m_nu( nu ),
				m_nv( nv ),
				m_uPeriodic( uPeriodic ),
				m_vPeriodic( vPeriodic )
		{
			m_uPatches = ( uPeriodic ) ? nu : nu - 1;
			m_vPatches = ( vPeriodic ) ? nv : nv - 1;
		}
		virtual	~CqSurfacePatchMeshBilinear();

		virtual	void	SetDefaultPrimitiveVariables( bool bUseDef_st = true )
		{}

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqSurfacePatchMeshBilinear");
		}
#endif

		virtual	void	Bound(CqBound* bound) const;
		virtual	CqMicroPolyGridBase* Dice()
		{
			return ( 0 );
		}
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );
		virtual bool	Diceable(const CqMatrix& /*matCtoR*/)
		{
			return ( false );
		}

		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual bool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( false );
		}

		virtual	TqUint	cUniform() const
		{
			return ( m_uPatches * m_vPatches );
		}
		virtual	TqUint	cVarying() const
		{
			return ( ( ( m_uPeriodic ) ? m_uPatches : m_uPatches + 1 ) * ( ( m_vPeriodic ) ? m_vPatches : m_vPatches + 1 ) );
		}
		virtual	TqUint	cVertex() const
		{
			return ( m_nu * m_nv );
		}
		virtual	TqUint	cFaceVarying() const
		{
			return ( cVarying() );
		}
		virtual CqSurface* Clone() const;

		virtual CqVector3D	SurfaceParametersAtVertex( TqInt index )
		{
			CqVector3D	vec( 0, 0, 0 );
			TqFloat u = static_cast<TqFloat>( index % m_nu );
			u /= ( m_nu - 1 );
			TqFloat v = static_cast<TqFloat>( index / m_nu );
			v /= ( m_nv - 1 );
			vec.x( u );
			vec.y( v );
			return ( vec );
		}

	protected:

		TqInt	m_uPatches,   			///< Number of patches in u.
		m_vPatches;			///< Number of patches in v.
		TqInt	m_nu,   				///< Number of control points in u.
		m_nv;				///< Number of control points in v.
		bool	m_uPeriodic,   		///< Is patches mesh periodic in u?
		m_vPeriodic;		///< Is patches mesh periodic in v?
}
;


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !PATCH_H_INCLUDED
